//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <thread>
#include <mutex>
#include "Scene.hpp"
#include "Renderer.hpp"

inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

std::mutex mtx;
const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.

void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    // change the spp value to change sample ammount
    int spp = 64;
    std::cout << "SPP: " << spp << "\n";
    const int th_num = 24;
    std::thread th[th_num];
    int progress = 0;
    auto render = [&](uint32_t start, uint32_t end) {
        for (uint32_t j = start; j < end; ++j) {
            for (uint32_t i = 0; i < scene.width; ++i) {
                // generate primary ray direction
                float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                    imageAspectRatio * scale;
                float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));
                for (int k = 0; k < spp; k++) {
                    framebuffer[(int)(j * scene.width + i)] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
                }
            }
            mtx.lock();
            progress++;
            UpdateProgress(progress / (float)scene.height);
            mtx.unlock();
        }
    };
    for (int i = 0; i < th_num; i++) {
        uint32_t start = (float)i / th_num * scene.height;
        uint32_t end = (float)(i + 1) / th_num * scene.height;
        th[i] = std::thread(render, start, end);
    }
    for (int i = 0; i < th_num; i++)
        th[i].join();
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
