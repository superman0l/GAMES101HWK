cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

# set(OpenCV_DIR D:/opencv/opencv/build) 放opencv库的
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
