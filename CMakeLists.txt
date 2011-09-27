project(kdevkernel)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

set(kdev_kernel_plugin_SRCS
    kernelplugin.cpp
)

kde4_add_plugin(kdevkernel
    ${kdev_kernel_plugin_SRCS})

target_link_libraries(kdevkernel
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
)

install(
    TARGETS kdevkernel
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
    FILES kdevkernel.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
