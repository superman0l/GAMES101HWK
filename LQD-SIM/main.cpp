#include <Eigen/SVD>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <opencv2/highgui/highgui_c.h>

#include "render.hpp"
#include "fluid_system.hpp"

int orix, oriy;
std::vector<Eigen::Vector3f> ori_rect;
std::vector<Eigen::Vector3f> rect;
bool isdrag = false;

SPH::FluidSystem fluidsys = SPH::FluidSystem();

void MouseEvent(int event, int x, int y, int flags, void* data)
{

    char text[30];
    cv::Mat tempImage;
    if (event == CV_EVENT_LBUTTONDOWN) //×ó¼üÂäÏÂ
    {
        orix = x; oriy = y;
        ori_rect = rect;
        isdrag = true;

    }
    else if (event == CV_EVENT_LBUTTONUP)//×ó¼üÉÏ
    {
        isdrag = false;
    }
    Eigen::Vector2f a;
    if (isdrag && flags == CV_EVENT_FLAG_LBUTTON) {
        int delta_x = x - orix; int delta_y = y - oriy;
        for (int i = 0; i < 4; i++) {
            rect[i][0] = ori_rect[i][0] + delta_x;
            rect[i][1] = ori_rect[i][1] - delta_y;
        }
        float mx = ((float)delta_x / 700) * (fluidsys.getBound().Max[0] - fluidsys.getBound().Min[0]),
            my = ((float)delta_y / 700) * (fluidsys.getBound().Max[1] - fluidsys.getBound().Min[1]);
        float delta_t2 = 0.03f * 0.03f;
        a = { -mx / delta_t2,-my / delta_t2 };
        fluidsys.addForce(0.4 * a);
    }
    else {
        fluidsys.setForce(Eigen::Vector2f(0.f, -9.8f));
    }
}

int main(int argc, const char** argv)
{
    float angle = 0;
    std::string filename = "output.png";
    
    rst::rasterizer r(700, 700);
    
    rect = { {200, 200, 0}, {200, 500, 0}, {500, 500, 0}, {500, 200, 0} };

    int key = 0;
    int frame_count = 0;

    fluidsys.init();
    fluidsys.addForce(Eigen::Vector2f(0.f, -9.8f));


    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.draw(rect, fluidsys);
        fluidsys.Update();

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        cv::setMouseCallback("image", MouseEvent);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

    }

    return 0;
}
