cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Result VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    include/result/result.hpp
)
# CMake is stupid and doesn't know which language this library is written in
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "include/result/result.hpp"
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
)
