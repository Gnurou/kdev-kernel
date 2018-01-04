cmake_minimum_required(VERSION 3.0.0)

project(kdevkernel)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# TODO Check these min versions
set(MIN_KDEV_VERSION "5.1.80")
set(MIN_QT_VER "5.5.0")
set(MIN_KF_VER "5.15.0")

# Find what is the prefix of the KDevelop installation
set(KDevelop_Include_DIR "include/kdevelop")
find_path(KDevelop_PREFIX ${KDevelop_Include_DIR})
set(KDevelop_Include_Full_DIR ${KDevelop_PREFIX}/${KDevelop_Include_DIR})
message(STATUS "KDevelop headers       : " ${KDevelop_Include_Full_DIR})
					
set(KDevPlatform_Include_DIR "include/kdevplatform")
set(KDevPlatform_Include_Full_DIR ${KDevelop_PREFIX}/${KDevPlatform_Include_DIR})
message(STATUS "KDevPlatform headers   : " ${KDevPlatform_Include_Full_DIR})

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 ${MIN_QT_VER} CONFIG REQUIRED Core Gui Widgets Test)
find_package(KF5 ${MIN_KF_VER} REQUIRED COMPONENTS ItemModels TextEditor)
find_package(KDevelop ${MIN_KDEV_VERSION} REQUIRED)
find_package(KDevPlatform ${MIN_KDEV_VERSION} REQUIRED CONFIG)

include_directories(
    ${KDevPlatform_Include_Full_DIR}
    ${KDevelop_Include_Full_DIR}
    ${KDevPlatform_DIR}
    ${KDevelop_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

set(kdevkernel_plugin_SRCS
    kdevkernelplugin.cpp

    config/projectconfigpage.cpp
)

ki18n_wrap_ui(kdevkernel_plugin_SRCS
    config/projectconfigpage.ui
)

kconfig_add_kcfg_files(kdevkernel_plugin_SRCS
    config/projectconfig.kcfgc
)

kdevplatform_add_plugin(kdevkernel JSON kdevkernel.json SOURCES ${kdevkernel_plugin_SRCS})

target_link_libraries(kdevkernel
    KDev::Interfaces
    KDev::Project
    KDev::OutputView
    KDev::Util
)

add_subdirectory(icons)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
