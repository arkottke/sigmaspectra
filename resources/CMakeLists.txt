
# Desktop entry
configure_file(sigmaspectra.cmake.desktop sigmaspectra.desktop @ONLY)

if (UNIX)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sigmaspectra.desktop
        DESTINATION .
        )
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/images/application-icon.svg
        DESTINATION .
        RENAME sigmaspectra.svg
        )
endif()
