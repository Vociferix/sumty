string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_CAPS)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL ON)
else()
    set(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL OFF)
endif()

set(_${PROJECT_NAME_CAPS}_DEVEL OFF)
if(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL)
    option(DEVEL "Enable option defaults for development" OFF)
    set(_${PROJECT_NAME_CAPS}_DEVEL ${DEVEL})
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(${PROJECT_NAME_CAPS}_INSTALL "Eanble installation of ${PROJECT_NAME}" ${${PROJECT_NAME_CAPS}_IS_TOPE_LEVEL})
option(${PROJECT_NAME_CAPS}_BUILD_TESTS "Enable building ${PROJECT_NAME} tests" ${_${PROJECT_NAME_CAPS}_DEVEL})

include(GNUInstallDirs)

add_library(${PROJECT_NAME}-warnings INTERFACE)
add_library(${PROJECT_NAME}-settings INTERFACE)

if(${${PROJECT_NAME_CAPS}_IS_TOP_LEVEL})
    include("${CMAKE_CURRENT_LIST_DIR}/format.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/doxygen.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/warnings.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/lints.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/coverage.cmake")
endif()

target_link_libraries(${PROJECT_NAME}-settings INTERFACE ${PROJECT_NAME}-warnings)
