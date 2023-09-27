string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_CAPS)

option(${PROJECT_NAME_CAPS}_DEVEL "Enable option defaults for development" OFF)

option(${PROJECT_NAME_CAPS}_INSTALL "Eanble installation of ${PROJECT_NAME}" ON)
option(${PROJECT_NAME_CAPS}_BUILD_TESTS "Enable building ${PROJECT_NAME} tests"
       ${${PROJECT_NAME_CAPS}_DEVEL})

include(GNUInstallDirs)

if(${PROJECT_NAME_CAPS}_DEVEL)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install" CACHE PATH "Installation prefix path")
endif()

add_library(${PROJECT_NAME}-warnings INTERFACE)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    include("${CMAKE_CURRENT_LIST_DIR}/format.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/doxygen.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/warnings.cmake")
endif()

add_library(${PROJECT_NAME}-settings INTERFACE)
target_link_libraries(${PROJECT_NAME}-settings INTERFACE ${PROJECT_NAME}-warnings)
