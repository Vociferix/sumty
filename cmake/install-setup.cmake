include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

if(NOT EXISTS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in")
    file(
        WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in"
        [[@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake")
]])
endif()

configure_package_config_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE "${PROJECT_NAME}::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    COMPONENT devel)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

install(
    FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
          "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    COMPONENT devel)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/sumty"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT devel)

install(
    FILES "${PROJECT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    COMPONENT devel)

install(
    DIRECTORY "${PROJECT_BINARY_DIR}/html"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    COMPONENT docs
    OPTIONAL)

set(CPACK_PACKAGE_VENDOR "Jack A Bernard Jr. <jack.a.bernard.jr@gmail.com>")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
include(CPack)
