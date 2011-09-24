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

kde4_add_plugin(kdevkernelplugin ${kdev_kernel_plugin_SRCS})

target_link_libraries(kdevprojectviewplugin ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevprojectviewplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kdevkernelplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})