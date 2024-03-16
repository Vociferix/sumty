find_package(Doxygen COMPONENTS dot)
if(NOT DOXYGEN_FOUND)
    return()
endif()

include(FetchContent)
FetchContent_Declare(DoxygenAwesome
    GIT_REPOSITORY "https://github.com/jothepro/doxygen-awesome-css"
    GIT_TAG "v2.3.2"
)
FetchContent_MakeAvailable(DoxygenAwesome)

set(DOXYGEN_GENERATE_MAN NO)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_DISABLE_INDEX NO)
set(DOXYGEN_FULL_SIDEBAR NO)
set(DOXYGEN_HTML_EXTRA_STYLESHEET "${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css")
set(DOXYGEN_HTML_COLORSTYLE LIGHT)
set(DOXYGEN_EXCLUDE_SYMBOLS detail)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_STRIP_FROM_PATH include/)
set(DOXYGEN_HIDE_SCOPE_NAMES YES)
set(DOXYGEN_PREDEFINED DOXYGEN)
set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
set(DOXYGEN_CLANG_DATABASE_PATH "${PROJECT_BINARY_DIR}")
doxygen_add_docs(docs "${PROJECT_SOURCE_DIR}/include")
