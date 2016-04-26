cmake_minimum_required(VERSION 2.8.12)

project(kdevkernel)

find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(KDEVELOP_VERSION "4.90.92")

find_package(KF5 "5.4.0" REQUIRED COMPONENTS
  Parts
  TextEditor
  ItemModels
)

find_package(KDevPlatform ${KDEVELOP_VERSION} REQUIRED)
find_package(KDevelop ${KDEVELOP_VERSION} REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/../kdevelop
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_PREFIX_PATH}/include/kdevelop
)

set(kdevkernel_plugin_SRCS
    kdevkernelplugin.cpp
)
kdevplatform_add_plugin(kdevkernel JSON kdevkernel.json SOURCES ${kdevkernel_plugin_SRCS})

target_link_libraries(kdevkernel
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
    KDev::Serialization
)

#add_subdirectory(icons)
