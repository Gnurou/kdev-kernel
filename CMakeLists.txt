project(kdevkernel)

find_package(KDE4 4.7.0 REQUIRED)
find_package(KDevPlatform 1.2.60 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

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

kde4_add_plugin(kdevkernel
    ${kdevkernel_plugin_SRCS})
target_link_libraries(kdevkernel
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
)

set (kdevkernel_kcm_SRCS
    kcm_kdevkernel.cpp
    kdevkernelconfigwidget.cpp
)

kde4_add_ui_files(kdevkernel_kcm_SRCS kdevkernelconfigwidget.ui)
kde4_add_kcfg_files(kdevkernel_kcm_SRCS kcfg_kdevkernelconfig.kcfgc)
kde4_add_plugin(kcm_kdevkernel ${kdevkernel_kcm_SRCS})
target_link_libraries(kcm_kdevkernel ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES})

install(
    TARGETS kdevkernel
            kcm_kdevkernel
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
    FILES kdevkernel.desktop
          kcm_kdevkernel.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)

add_subdirectory(icons)
