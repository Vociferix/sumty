find_package(Doxygen COMPONENTS dot)
if(NOT DOXYGEN_FOUND)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    doxygen_awesome_css
    GIT_REPOSITORY "https://github.com/jothepro/doxygen-awesome-css.git"
    GIT_TAG v2.2.1)

FetchContent_MakeAvailable(doxygen_awesome_css)

set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_HTML_EXTRA_STYLESHEET
    "${doxygen_awesome_css_SOURCE_DIR}/doxygen-awesome.css"
    "${doxygen_awesome_css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css")
set(DOXYGEN_EXCLUDE_SYMBOLS detail)
doxygen_add_docs(docs include)
