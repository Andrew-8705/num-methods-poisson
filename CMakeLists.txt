cmake_minimum_required(VERSION 3.16)
project(PoissonSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp" "src/Problems/*.cpp" "src/Solvers/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})