option(LINTS "Enable linters by default" ${DEVEL})

option(CLANG_TIDY_LINTS "Enable clang-tidy lints" ${LINTS})
if(CLANG_TIDY_LINTS)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CLANG_TIDY_CMD
            "${CLANG_TIDY}" -extra-arg=-Wno-unknown-warning-option
            -extra-arg=-Wno-ignored-optimization-argument
            -extra-arg=-Wno-unused-command-line-argument)
        if(WERROR)
            list(APPEND CLANG_TIDY_CMD -warnings-as-errors=*)
        endif()
    else()
        message(
            WARNING
                "clang-tidy lints were enabled, but no clang-tidy executable was found. clang-tidy lints will not be checked."
        )
    endif()
endif()

option(CPPCHECK_LINTS "Enable cppcheck lints" ${LINTS})
if(CPPCHECK_LINTS)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
            set(CPPCHECK_TEMPLATE "vs")
        else()
            set(CPPCHECK_TEMPLATE "gcc")
        endif()
        set(CPPCHECK_CMD
            "${CPPCHECK}"
            --template=${CPPCHECK_TEMPLATE}
            --enable=style,performance,warning,portability
            --inline-suppr
            # We cannot act on a bug/missing feature of cppcheck
            --suppress=cppcheckError
            --suppress=internalAstError
            # if a file does not have an internalAstError, we get an unmatchedSuppression
            # error
            --suppress=unmatchedSuppression
            # noisy and incorrect sometimes
            --suppress=passedByValue
            # ignores code that cppcheck thinks is invalid C++
            --suppress=syntaxError
            --suppress=preprocessorErrorDirective
            # handled somewhat better by clang-tidy
            --suppress=noExplicitConstructor
            --inconclusive
            --std=c++20)
        if(WERROR)
            list(APPEND CPPCHECK_CMD --error-exitcode=2)
        endif()
    else()
        message(
            WARNING
                "cppcheck lints were enabled, but no cppcheck executable was found. cppcheck lints will not be checked."
        )
    endif()
endif()

option(IWYU_LINTS "Enable include-what-you-use lints" ${LINTS})
if(IWYU_LINTS)
    find_program(IWYU include-what-you-use)
    if(IWYU)
        set(IWYU_CMD
            "${IWYU}"
            -Wno-unknown-warning-option
            -Wno-ignored-optimization-argument
            -Wno-unused-command-line-argument
            -Xiwyu
            --cxx17ns
            -Xiwyu
            --no_fwd_decls)
        if(WERROR)
            list(APPEND IWYU_CMD -Xiwyu --error=2)
        endif()
    else()
        message(
            WARNING
                "include-what-you-use lints were enabled, but no include-what-you-use executable was found. include-what-you-use lints will not be checked."
        )
    endif()
endif()

function(${PROJECT_NAME}_enable_lints TGT)
    if(CLANG_TIDY_CMD)
        set_property(TARGET "${TGT}" PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_CMD})
    endif()
    if(CPPCHECK_CMD)
        set_property(TARGET "${TGT}" PROPERTY CXX_CPPCHECK ${CPPCHECK_CMD})
    endif()
    if(IWYU_CMD)
        set_property(TARGET "${TGT}" PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_CMD})
    endif()
endfunction()
