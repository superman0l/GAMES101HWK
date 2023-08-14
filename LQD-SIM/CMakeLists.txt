cmake_minimum_required(VERSION 3.10)
project(LQD_SIM)

# set(OpenCV_DIR D:/opencv/opencv/build)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)

add_executable(LQD_SIM main.cpp render.hpp render.cpp fluid_system.hpp fluid_system.cpp fluid_grid.hpp fluid_grid.cpp fluid_buffer.hpp fluid_buffer.cpp)
target_link_libraries(LQD_SIM ${OpenCV_LIBRARIES})