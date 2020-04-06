find_package(gtest REQUIRED)

add_executable(Epstl_test main.cpp
    vectorTest.cpp vectorTest.hpp
    mapTest.cpp mapTest.hpp
    quadtreeTest.cpp quadtreeTest.hpp
    quadtreeRegionTest.cpp quadtreeRegionTest.hpp
    mathTest.cpp mathTest.hpp
    workTest.cpp workTest.hpp
    workerThreadTest.cpp workerThreadTest.hpp
    geometryToolsTest.cpp
    pipelineTest.cpp pipelineTest.hpp)
target_link_libraries(Epstl_test gtest epstl)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")
add_custom_command(TARGET Epstl_test POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_CURRENT_SOURCE_DIR}/data"
     "${CMAKE_CURRENT_BINARY_DIR}/data"
  COMMENT "Copying test data output directory")
endif()
