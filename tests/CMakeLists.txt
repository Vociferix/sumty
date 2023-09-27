include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG v3.4.0)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)

add_executable(tests option.cpp result.cpp variant.cpp)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain ${PROJECT_NAME}::${PROJECT_NAME}
                                    ${PROJECT_NAME}-settings)

if(COMMAND ${PROJECT_NAME}_enable_lints)
    cmake_language(CALL ${PROJECT_NAME}_enable_lints tests)
endif()

catch_discover_tests(tests)
