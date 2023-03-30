cmake_minimum_required(VERSION 3.10)

# set the project name
project(2D_Fluid_Sim VERSION 0.0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set startup project for sln files
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(p8g)

add_executable(${PROJECT_NAME} hello_rectangle.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${p8g_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${p8g_LIBRARIES})

#target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
