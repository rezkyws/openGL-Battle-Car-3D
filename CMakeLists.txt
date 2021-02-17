cmake_minimum_required(VERSION 3.17)
project(uas)

set(CMAKE_CXX_STANDARD 14)

add_executable(uas main.cpp BmpLoader.cpp)
target_link_libraries(${PROJECT_NAME} -lglut -lGLU -lGL)