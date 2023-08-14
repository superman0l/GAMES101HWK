#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/SVD>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(Eigen::Vector3f axis, float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    //for triangle rotate
    float a = fmod(rotation_angle, 360.f) * MY_PI / 180;
    //ÈÆzÖá
    model << cos(a), -sin(a), 0, 0,
        sin(a), cos(a), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    Eigen::Matrix3f n = Eigen::Matrix3f::Identity();
    n<< 0, -1 * axis[2], axis[1], axis[2], 0, -1 * axis[0], -1 * axis[1], axis[0], 0;
    Eigen::MatrixXf m3;
    m3 = cos(a) * Eigen::Matrix3f::Identity() + (1 - cos(a)) * axis * axis.transpose() + sin(a) * n;
    m3.conservativeResize(4, 4);
    m3.col(3).setZero();
    m3.row(3).setZero();
    m3(3, 3) = 1;
    model = m3;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    //for perspective that needs 2 matrics
    Eigen::Matrix4f squish = Eigen::Matrix4f::Identity(); 
    float zn = -zNear, zf = -zFar;
    squish << zn, 0, 0, 0,
        0, zn, 0, 0,
        0, 0, zn + zf, -zn * zf,
        0, 0, 1, 0;
    Eigen::Matrix4f scale = Eigen::Matrix4f::Identity();
    float t = tan(eye_fov / 2) * zNear;
    float tx = t * aspect_ratio;
    scale << 1/tx, 0, 0, 0,
        0, 1/t, 0, 0,
        0, 0, 2/fabs(zf-zn), 0,
        0, 0, 0, 1;
    Eigen::Matrix4f M;
    M << 1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, -1, 0, 
        0, 0, 0, 1;
    
    projection << M * scale * squish;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    std::cout << "Input the axis:(x y z)" << std::endl;
    float x, y, z;
    std::cin >> x >> y >> z;
    Eigen::Vector3f axis = { 0,0,0 };
    axis << x, y, z;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
