// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector3f A, B, C, D;
    A[0] = _v[0][0]; A[1] = _v[0][1]; B[0] = _v[1][0]; B[1] = _v[1][1]; C[0] = _v[2][0]; C[1] = _v[2][1]; A[2] = 0; B[2] = 0; C[2] = 0;
    D[0] = x; D[1] = y; D[2] = 0;
    Vector3f AB = B - A, AD = D - A;
    Vector3f BC = C - B, BD = D - B;
    Vector3f CA = A - C, CD = D - C;

    if (AB.cross(AD)[2]>0 && BC.cross(BD)[2]>0 && CA.cross(CD)[2]>0)return true;
    else return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);

        //SSAA reaterize
        /*
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Eigen::Vector3f color = { 0.,0.,0. };
                Vector3f point = { (float)i, (float)j, 0. };//already used SSAA to approach the z-axis problem so here just use 0 to draw
                for (float n = start_point_per_pixel; n < 1; n+=pixel_sample_size) {
                    for (float m = start_point_per_pixel; m < 1; m+=pixel_sample_size) {
                        color += ssaa_frame_buf[get_ssaa_index(i, j, n, m)];
                    }
                }
                set_pixel(point, color/(ssaa*ssaa));
            }
        }*/
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    float xmin=INT_MAX, ymin=INT_MAX, xmax=INT_MIN, ymax=INT_MIN;
    for (int i = 0; i < 3; i++) {
        if (t.v[i][0] < xmin)
            xmin = t.v[i][0];
        if (t.v[i][0] > xmax)
            xmax = t.v[i][0];
        if (t.v[i][1] < ymin)
            ymin = t.v[i][1];
        if (t.v[i][1] > ymax)
            ymax = t.v[i][1];
    }
    
    for (int i = xmin; i <= xmax; i++) {
        for (int j = ymin; j <= ymax; j++) {


            //SSAA anti-aliasing
            /*
            for (float n = start_point_per_pixel; n < 1; n+=pixel_sample_size) {
                for (float m = start_point_per_pixel; m < 1; m+=pixel_sample_size) {
                    if (insideTriangle((float)i - 1/ssaa + n, (float)j - 1/ssaa + m, t.v)) {

                        auto [alpha, beta, gamma] = computeBarycentric2D((float)i - 1/ssaa + n, (float)j - 1/ssaa + m, t.v);
                        float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;

                        if (z_interpolated < ssaa_depth_buf[get_ssaa_index(i, j, n, m)]) {
                            int id = get_ssaa_index(i, j, n, m);
                            ssaa_depth_buf[id] = z_interpolated;
                            ssaa_frame_buf[id] = t.getColor();
                        }
                    }
                }
            }*/

            //MSAA anti-aliasing
            //consider how to fix the black-edge problem without enlarging the vector
            /*
            int index = get_index(i, j);
            float count = 0.0;
            float max_count = ssaa * ssaa;
            // iterate through the sampling points
            for (float n = start_point_per_pixel; n < 1; n += pixel_sample_size) {
                for (float m = start_point_per_pixel; m < 1; m += pixel_sample_size) {
                    if (insideTriangle((float)i - 1 / ssaa + n, (float)j - 1 / ssaa + m, t.v)) {
                        count += 1.0;
                    }
                }
            }
            if (count == 0)continue;
            auto [alpha, beta, gamma] = computeBarycentric2D(i, j, t.v);
            float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;

            if (z_interpolated < depth_buf[index]) {
                Eigen::Vector3f p;
                p << i, j, z_interpolated;
                Eigen::Vector3f color = t.getColor() * (count / max_count) + (1 - count / max_count) * frame_buf[index];
                if (count != max_count) {
                    msaa_check_edge[index] = count / max_count;
                }
                set_pixel(p, color);
                depth_buf[index] = z_interpolated;
            }
            else {
                if (msaa_check_edge[index] != 1) {
                    Eigen::Vector3f color = t.getColor() * (1 - msaa_check_edge[index]) + frame_buf[index] * msaa_check_edge[index];
                    Eigen::Vector3f p;
                    p << i, j, z_interpolated;
                    set_pixel(p, color);
                }
            }
            */


            //no anti-aliasing
            
            if (insideTriangle(i, j, t.v)) {
                auto [alpha, beta, gamma] = computeBarycentric2D(i, j, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                Vector3f point = { (float)i, (float)j, z_interpolated };
                if (z_interpolated < depth_buf[get_index(i, j)]) {
                    set_pixel(point, t.getColor());
                    depth_buf[get_index(i, j)] = z_interpolated;
                }
            }

        }
    }

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        fill(ssaa_frame_buf.begin(), ssaa_frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
        fill(msaa_check_edge.begin(), msaa_check_edge.end(), 1);
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        fill(ssaa_depth_buf.begin(), ssaa_depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    ssaa_depth_buf.resize(w * h * ssaa * ssaa);//we set 2*2
    ssaa_frame_buf.resize(w * h * ssaa * ssaa);
    msaa_check_edge.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

int rst::rasterizer::get_ssaa_index(int x, int y, float i, float j) {
    
    int yy = y * ssaa + (int)(j / pixel_sample_size);
    int xx = x * ssaa + (int)(i / pixel_sample_size);
    return (height * ssaa - 1 - yy) * width * ssaa + xx;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on