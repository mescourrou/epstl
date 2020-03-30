cmake_minimum_required(VERSION 3.10)
project(epstl)

option(USE_CUSTOM_STL "Use the custom implementation of the STL" ON)
option(EPSTL_BUILD_TEST "Build the tests" ON)

set(CPP_VERSION 17)

set(CMAKE_CXX_STANDARD ${CPP_VERSION})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(EPSTL_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty")


add_library(epstl INTERFACE)
add_dependencies(epstl CONFIG_FILE)
target_include_directories(epstl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(USE_CUSTOM_STL)
    target_compile_definitions(epstl INTERFACE USE_CUSTOM_STL)
endif()

if (EPSTL_BUILD_TEST)
    find_package(gtest REQUIRED)
    target_link_libraries(epstl INTERFACE gtest)
    add_subdirectory(unit_tests)
endif()
