cmake_minimum_required(VERSION 3.16)

project(stack_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_subdirectory(include)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
