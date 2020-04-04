INCLUDE(ExternalProject)
if (NOT TARGET epstl)

    ExternalProject_Add(
        epstl
        DOWNLOAD_COMMAND ""
        SOURCE_DIR "${THIRDPARTY_DIR}/epstl"
        PREFIX "${CMAKE_BINARY_DIR}/epstl"
        CMAKE_ARGS -DUSE_CUSTOM_STL=OFF
        INSTALL_COMMAND ""
        )
    ExternalProject_Get_Property(epstl source_dir)
      set(EPSTL_INCLUDE_DIRS ${source_dir}/include)

endif()
