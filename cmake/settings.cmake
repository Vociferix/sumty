string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_CAPS)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL TRUE)
else()
    set(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL FALSE)
endif()

if(${PROJECT_NAME_CAPS}_IS_TOP_LEVEL)
    option(DEVEL "Enable option defaults for development" OFF)
else()
    set(DEVEL OFF)
endif()

option(${PROJECT_NAME_CAPS}_INSTALL "Eanble installation of ${PROJECT_NAME}" ON)
option(${PROJECT_NAME_CAPS}_BUILD_TESTS "Enable building ${PROJECT_NAME} tests" ${DEVEL})

include(GNUInstallDirs)

add_library(${PROJECT_NAME}-warnings INTERFACE)

if(${${PROJECT_NAME_CAPS}_IS_TOP_LEVEL})
    include("${CMAKE_CURRENT_LIST_DIR}/format.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/doxygen.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/warnings.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/lints.cmake")
endif()

add_library(${PROJECT_NAME}-settings INTERFACE)
target_link_libraries(${PROJECT_NAME}-settings INTERFACE ${PROJECT_NAME}-warnings)
