cmake_minimum_required(VERSION 3.21)
project(Cutline VERSION 1.4.0 LANGUAGES CXX)
if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Network)

set(CUTLINE_SOURCES
    src/main.cpp
    src/model.h
    src/model.cpp
    src/host.h
    src/theme.h
    src/theme.cpp
    src/i18n.h
    src/i18n.cpp
    src/icons.h
    src/icons.cpp
    src/overlay.h
    src/overlay.cpp
    src/timeline.h
    src/timeline.cpp
    src/videoview.h
    src/videoview.cpp
    src/dialogs.h
    src/dialogs.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/updater.h
    src/updater.cpp
    src/textrender.h
    src/textrender.cpp
    assets/fonts.qrc
)

if(WIN32)
    add_executable(cutline WIN32 ${CUTLINE_SOURCES} assets/app.rc)
    set_target_properties(cutline PROPERTIES OUTPUT_NAME Cutline)
elseif(APPLE)
    set(CUTLINE_ICNS ${CMAKE_SOURCE_DIR}/assets/icon.icns)
    set_source_files_properties(${CUTLINE_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    add_executable(cutline MACOSX_BUNDLE ${CUTLINE_SOURCES} ${CUTLINE_ICNS})
    set_target_properties(cutline PROPERTIES
        OUTPUT_NAME Cutline
        MACOSX_BUNDLE_BUNDLE_NAME Cutline
        MACOSX_BUNDLE_GUI_IDENTIFIER io.github.cutline
        MACOSX_BUNDLE_ICON_FILE icon.icns
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_INFO_STRING "Cutline video editor"
        MACOSX_BUNDLE_COPYRIGHT "Apache License 2.0"
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/assets/Info.plist.in)
else()
    add_executable(cutline ${CUTLINE_SOURCES})
endif()

target_link_libraries(cutline PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Network)
target_compile_definitions(cutline PRIVATE APP_VERSION="${PROJECT_VERSION}")
if(MSVC)
    target_compile_options(cutline PRIVATE /utf-8 /W3 /Zc:__cplusplus)
endif()

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS cutline RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES assets/cutline.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES assets/icon.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME cutline.png)
endif()
