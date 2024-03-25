option(COVERAGE "Enable code coverage" OFF)
if(NOT COVERAGE)
    return()
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(SEND_ERROR "Code coverage is only supported under GCC")
    return()
endif()

find_program(GCOV gcov REQUIRED)
find_program(GCOVR gcovr REQUIRED)

add_compile_options(--coverage -fno-inline -fno-inline-small-functions -fno-default-inline)
link_libraries(--coverage gcov)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fprofile-abs-path HAVE_PROFILE_ABS_PATH)
if(HAVE_PROFILE_ABS_PATH)
    add_compile_options(-fprofile-abs-path)
    link_libraries(-fprofile-abs-path)
endif()

add_custom_target(coverage
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/coverage"
    COMMAND "${GCOVR}"
        --root="${PROJECT_SOURCE_DIR}"
        --object-directory="${PROJECT_BINARY_DIR}"
        --filter="${PROJECT_SOURCE_DIR}/include"
        --branches
        --decisions
        --txt
        --html coverage/index.html
        --html-details
        --json coverage.json
        --json-pretty
        --json-summary coverage-summary.json
        --json-summary-pretty
        --csv coverage.csv
        --xml coverage.xml
        --sonarqube coverage-sonarqube.xml
        --coveralls coverage-coveralls.json
        --coveralls-pretty
        --gcov-executable "${GCOV}"
        --gcov-ignore-parse-errors
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Generating coverage reports"
)
