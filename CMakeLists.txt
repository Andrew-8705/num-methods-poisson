cmake_minimum_required(VERSION 3.16)
project(PoissonSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp" "src/Qt/*.cpp" "src/Problems/*.cpp" "src/Solvers/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)