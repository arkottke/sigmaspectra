###
# Copyright (c) 2009, Paul Gideon Dann
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
###

# Ensure that WiX is in the path
find_path(WIX_PATH candle.exe)
if (NOT WIX_PATH)
    message(WARNING "Unable to find WiX in the PATH. The WiX installer will be disabled.")
endif()

# Ensure that 7z is in the path
find_path(7Z_PATH 7z.exe)
if (NOT 7Z_PATH)
    message(WARNING "Unable to find 7z in the PATH. The zip archive will be disabled.")
endif()

# Configure the installer scripts
configure_file(installer.cmake.nsi ${CMAKE_BINARY_DIR}/installer.nsi @ONLY)
configure_file(installer.cmake.wxs installer.wxs @ONLY)

# Compiler Runtime DLLs
if (MSVC)
    # Visual Studio
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP true)
    include(InstallRequiredSystemLibraries)
    install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION .)
else()
    # MinGW
    get_filename_component(MINGW_DLL_DIR ${CMAKE_CXX_COMPILER} PATH)
    install(FILES
        ${MINGW_DLL_DIR}/libgcc_s_dw2-1.dll
        ${MINGW_DLL_DIR}/libstdc++-6.dll
        DESTINATION .
        )
endif()

# Qt Libraries
get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
get_filename_component(QWT_DLL_DIR ${QWT_LIBRARY} PATH)
install(FILES
    ${QT_DLL_DIR}/libwinpthread-1.dll
    ${QT_DLL_DIR}/Qt5Core.dll
    ${QT_DLL_DIR}/Qt5Gui.dll
    ${QT_DLL_DIR}/Qt5OpenGL.dll
    ${QT_DLL_DIR}/Qt5PrintSupport.dll
    ${QT_DLL_DIR}/Qt5Svg.dll
    ${QT_DLL_DIR}/Qt5Widgets.dll
    ${QT_DLL_DIR}/Qt5Xml.dll
    ${QWT_DLL_DIR}/qwt.dll
    ${CMAKE_SOURCE_DIR}/win/qt.conf
    DESTINATION .
    )
# Qt Platform Plugin
install(FILES
    ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
    DESTINATION plugins/platforms
    )
# Manual and example
install(FILES
    ${CMAKE_SOURCE_DIR}/manual/manual.pdf
    ${CMAKE_SOURCE_DIR}/manual/example-target.csv
    DESTINATION .
    )
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/example 
    DESTINATION .
    )
# Custom target to build the WiX installer. Should be run after the install
# target.
if (WIX_PATH)
    add_custom_target(installer
        COMMAND ${WIX_PATH}/candle.exe ${CMAKE_CURRENT_BINARY_DIR}/installer.wxs
        COMMAND ${WIX_PATH}/light.exe -ext WixUIExtension -ext WixUtilExtension
        -b ${CMAKE_INSTALL_PREFIX} -b ${CMAKE_SOURCE_DIR}/win
        -out "${PROJECT_LONGNAME}-v${PROJECT_VERSION}-${PROJECT_GITHASH}.msi"
        installer.wixobj
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS install
        VERBATIM
        )
endif()

if (7Z_PATH)
    add_custom_target(archive
        COMMAND 7z a ${CMAKE_CURRENT_BINARY_DIR}/SigmaSpectra-v${PROJECT_VERSION}-${PROJECT_GITHASH}.zip *
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS install
        VERBATIM
        )
endif()
