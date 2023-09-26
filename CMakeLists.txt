cmake_minimum_required(VERSION 3.14)

project(
    sumty
    VERSION 0.1.0
    DESCRIPTION "Better optional data types for C++"
    HOMEPAGE_URL "https://github.com/vociferix/sumty"
    LANGUAGES CXX)

string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_CAPS)

option(${PROJECT_NAME_CAPS}_INSTALL "Eanble installation of ${PROJECT_NAME}" ON)
option(${PROJECT_NAME_CAPS}_BUILD_TESTS
    "Enable building ${PROJECT_NAME} tests" OFF)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

if(${PROJECT_NAME_CAPS}_INSTALL)
    include(CMakePackageConfigHelpers)

    target_include_directories(${PROJECT_NAME} INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    if(NOT EXISTS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in")
        file(WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in"
            [[@PACKAGE_INIT@

include(${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake)
]])
    endif()

    configure_package_config_file(
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        COMPONENT devel)

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets)

    install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        COMPONENT devel)

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/sumty"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT devel)

    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
        COMPONENT devel)

    set(CPACK_PACKAGE_VENDOR
        "Jack A Bernard Jr. <jack.a.bernard.jr@gmail.com>")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    include(CPack)
endif()

if(${PROJECT_NAME_CAPS}_BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()
