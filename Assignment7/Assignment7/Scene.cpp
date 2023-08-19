//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    
    if (depth > this->maxDepth) {
        return Vector3f(0.0, 0.0, 0.0);
    }
    Intersection intersection = Scene::intersect(ray);
    Material* m = intersection.m;
    if (!intersection.happened)return Vector3f(0.0, 0.0, 0.0);
    if (intersection.m->hasEmission()) {
        return m->getEmission();
    }
    Object* hitObject = intersection.obj;
    Vector3f hitColor = this->backgroundColor;
    auto p = intersection.coords, N = intersection.normal, wo = ray.direction;

    Intersection inter;
    float pdf_light;
    sampleLight(inter, pdf_light);
    auto x = inter.coords, ws = (x - p).normalized(), NN = inter.normal, emit = inter.emit;
    float distance = (x - p).norm(), error_range = -0.001;
    Ray r(p, ws);
    Intersection check = intersect(r);
    Vector3f L_dir;
    if(check.distance > distance+error_range) 
        L_dir = emit * m->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) / powf(distance,2) / pdf_light;
    Vector3f L_indir = 0.0;
    float roll = get_random_float();
    if (roll < RussianRoulette) {
        auto wi = m->sample(wo, N).normalized();
        Ray r(p, wi);
        Intersection hit = intersect(r);
        if (hit.happened && !hit.m->hasEmission())
            L_indir = castRay(r, depth+1) * m->eval(wo, wi, N) * dotProduct(wi, N) / m->pdf(wo, wi, N) / RussianRoulette;
    }
    return L_dir + L_indir;
}