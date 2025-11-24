cmake_minimum_required(VERSION 3.19)
project(DragonEditor LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(DragonEditor
        WIN32 MACOSX_BUNDLE
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        settings.ui
        DragonEditor.qrc
        SettingsDialog.cpp
        SettingsDialog.h
)

set(TRANSLATION_TS_FILES
        DragonEditor_zh_CN.ts
)

qt_add_translations(
        TARGETS DragonEditor
        TS_FILES ${TRANSLATION_TS_FILES}
)

target_link_libraries(DragonEditor
        PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS DragonEditor
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
        TARGET DragonEditor
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
