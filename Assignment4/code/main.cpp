#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 5) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    return cv::Point2f();

}

int c(int a, int b) {
    if (a == 0)return 1;
    int up = 1, down = 1, x = b, y = a;
    for (int i = 0; i < a; i++) {
        up *= x; x--;
        down *= y; y--;
    }
    return up / down;
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    int xmin=10000, xmax=-10000, ymin=10000, ymax=-10000;
    for (double t = 0.0; t <= 1.0; t += 0.0001)
    {
        cv::Point2f result_point;
        for (int i = 0; i < control_points.size(); i++) {
            result_point += control_points[i] * c(i, control_points.size() - 1) * pow(1 - t, control_points.size() - 1 - i) * pow(t, i);
        }
        
        window.at<cv::Vec3b>(result_point.y, result_point.x)[1] = 255;
        
        const float x[4] = { 0,0,1,-1};
        const float y[4] = { +1,-1,0,0 };
        int xnow = round(result_point.x), ynow = round(result_point.y);
        float d = std::sqrt(std::pow(result_point.x - xnow, 2) + std::pow(result_point.y - ynow, 2));
        for (int i = 0; i < 4; i++) {
            float x_neibor = floor(result_point.x + x[i]);
            float y_neibor = floor(result_point.y + y[i]);
            if (x_neibor >= 0 && x_neibor < 700 && y_neibor >= 0 && y_neibor < 700) {
                float w = d / std::sqrt((std::pow(x_neibor - result_point.x, 2) + std::pow(y_neibor - result_point.y, 2)));
                window.at<cv::Vec3b>(y_neibor, x_neibor)[1] = std::max(float(window.at<cv::Vec3b>(y_neibor, x_neibor)[1]), 255 * w);
            }
        }

    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 5) 
        {
            naive_bezier(control_points, window);
               bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
