string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_CAPS)

option(${PROJECT_NAME_CAPS}_INSTALL "Eanble installation of ${PROJECT_NAME}" ON)
option(${PROJECT_NAME_CAPS}_BUILD_TESTS "Enable building ${PROJECT_NAME} tests"
       OFF)

include(GNUInstallDirs)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    include("${CMAKE_CURRENT_LIST_DIR}/format.cmake")
endif()
