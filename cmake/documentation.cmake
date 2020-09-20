function(add_doc_target)
    find_program(doxygen doxygen)
    find_program(xdg-open xdg-open)

    if (NOT doxygen)
        message(WARNING "Cannot generate documentation without Doxygen")
        return()
    endif (NOT doxygen)

    # Configure Doxygen and adoc variables
    set (DOXYGEN_SRC_PATH          ${CMAKE_SOURCE_DIR}/src)

    set (DOXYGEN_DST_FILENAME ${CMAKE_BINARY_DIR}/Doxyfile)
    set (DOXYGEN_DST_PATH          ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set (DOXYGEN_FILENAME ${CMAKE_SOURCE_DIR}/docs/Doxyfile)

    # Generate Doxygen configuration file for current project
    configure_file(${DOXYGEN_FILENAME} ${DOXYGEN_DST_FILENAME} @ONLY)
    message(STATUS "The Doxygen configuration has been written in: ${DOXYGEN_DST_FILENAME}")

    # Generate Doxygen project documentation
    add_custom_target(doc_doxygen_clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_DST_PATH}
        COMMENT "Clear Doxygen documentation output folder")

    # Generate Doxygen project documentation
    add_custom_target(doc_doxygen
        DEPENDS doc_doxygen_clean
        COMMAND ${doxygen} ${DOXYGEN_DST_FILENAME}
        COMMENT "Generate ${PROJECT_NAME} Doxygen documentation")

    add_custom_target(doc
        DEPENDS doc_doxygen)

    if (xdg-open)
        # Helper command to open generated documentation on web browser using xdg-open
        add_custom_target(show_doc
            COMMAND ${xdg-open} "${DOXYGEN_DST_PATH}/html/index.html")
    endif (xdg-open)
endfunction()
