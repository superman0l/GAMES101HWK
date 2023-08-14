#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <Eigen/SVD>

#include "fluid_system.hpp"
using namespace Eigen;

namespace rst {
    enum class Buffers
    {
        Color = 1,
        Depth = 2
    };

    inline Buffers operator|(Buffers a, Buffers b)
    {
        return Buffers((int)a | (int)b);
    }

    inline Buffers operator&(Buffers a, Buffers b)
    {
        return Buffers((int)a & (int)b);
    }

    enum class Primitive
    {
        Line,
        Triangle
    };

    /*
     * For the curious : The draw function takes two buffer id's as its arguments.
     * These two structs make sure that if you mix up with their orders, the
     * compiler won't compile it. Aka : Type safety
     * */
    struct pos_buf_id
    {
        int pos_id = 0;
    };

    struct ind_buf_id
    {
        int ind_id = 0;
    };

    class rasterizer
    {
    public:
        rasterizer(int w, int h);
        pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
        ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);


        void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);

        void clear(Buffers buff);

        void draw(std::vector<Eigen::Vector3f> rect, SPH::FluidSystem sys);

        std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }

        void drawliquid(std::vector<Eigen::Vector3f> rect, SPH::FluidSystem sys);

    private:
        void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);

        void rasterize_drawrect(std::vector<Eigen::Vector3f>& rect);

    private:
        Eigen::Matrix4f model;
        Eigen::Matrix4f view;
        Eigen::Matrix4f projection;

        std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
        std::map<int, std::vector<Eigen::Vector3i>> ind_buf;

        std::vector<Eigen::Vector3f> frame_buf;
        std::vector<float> depth_buf;
        int get_index(int x, int y);

        int width, height;

        int next_id = 0;
        int get_next_id() { return next_id++; }
    };
}