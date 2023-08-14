//
// Created by goksu on 4/6/19.
//

#include "render.hpp"

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <stdexcept>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f>& positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return { id };
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i>& indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return { id };
}

// Bresenham's line drawing algorithm
// Code taken from a stack overflow answer: https://stackoverflow.com/a/16405254
void rst::rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end)
{
    auto x1 = begin.x();
    auto y1 = begin.y();
    auto x2 = end.x();
    auto y2 = end.y();

    Eigen::Vector3f line_color = { 255, 255, 255 };

    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;

    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = fabs(dx);
    dy1 = fabs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
        if (x <= 0 || x >= 700 || y <= 0 || y >= 700);
        else set_pixel(point, line_color);
        for (i = 0; x < xe; i++)
        {
            x = x + 1;
            if (px < 0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            //            delay(0);
            Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
            if (x <= 0 || x >= 700 || y <= 0 || y >= 700);
            else set_pixel(point, line_color);
        }
    }
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }
        Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
        if (x <= 0 || x >= 700 || y <= 0 || y >= 700);
        else set_pixel(point, line_color);
        for (i = 0; y < ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py = py + 2 * dx1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            //            delay(0);
            Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
            if (x <= 0 || x >= 700 || y <= 0 || y >= 700);
            else set_pixel(point, line_color);
        }
    }
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

void rst::rasterizer::drawliquid(std::vector<Eigen::Vector3f> rect, SPH::FluidSystem sys) {
    Eigen::Vector3f line_color = { 255, 255, 255 };
    float cx = 0.f, cy = 0.f;
    auto bf = sys.getBuffer();
    auto bd = sys.getBound();
    for (int i = 0; i < bf.countMolecule(); i++) {
        cx = bf.getMolecule(i).pos[0] / (bd.Max[0] - bd.Min[0]) * (rect[2][0] - rect[0][0]) + rect[0][0];
        cy = bf.getMolecule(i).pos[1] / (bd.Max[1] - bd.Min[1]) * (rect[2][1] - rect[0][1]) + rect[0][1];
        Eigen::Vector3f point = Eigen::Vector3f((int)cx, (int)cy, 1.0f);
        set_pixel(point, line_color);
    }
}

void rst::rasterizer::draw(std::vector<Eigen::Vector3f> rect, SPH::FluidSystem sys)
{
    rasterize_drawrect(rect);
    drawliquid(rect, sys);
}

void rst::rasterizer::rasterize_drawrect(std::vector<Eigen::Vector3f>& rect)
{
    for(int i=0;i<4;i++)
        draw_line(rect[i%4], rect[(i+1)%4]);
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    if (point.x() <= 0 || point.x() >= width ||
        point.y() <= 0 || point.y() >= height) return;
    auto ind = (height - point.y()) * width + point.x();
    frame_buf[ind] = color;
}

