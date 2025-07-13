cmake_minimum_required(VERSION 3.8)
project(fcitx5-paste-primary)

find_package(ECM REQUIRED 1.0.0)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(PkgConfig REQUIRED)
find_package(Gettext REQUIRED)

include(FeatureSummary)
include(GNUInstallDirs)
include(ECMSetupVersion)
include(ECMUninstallTarget)

find_package(Threads REQUIRED)
find_package(Fcitx5Core 5.0 REQUIRED)
find_package(Fcitx5Module REQUIRED COMPONENTS XCB)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-paste-primary\")
fcitx5_add_i18n_definition()

add_subdirectory(po)
add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
