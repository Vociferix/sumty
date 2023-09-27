cmake_minimum_required(VERSION 3.14)

project(
    sumty
    VERSION 0.1.0
    DESCRIPTION "Better optional data types for C++"
    HOMEPAGE_URL "https://github.com/vociferix/sumty"
    LANGUAGES CXX)

include(cmake/settings.cmake)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

if(${PROJECT_NAME_CAPS}_INSTALL)
    include(cmake/install-setup.cmake)
endif()

if(${PROJECT_NAME_CAPS}_BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()
