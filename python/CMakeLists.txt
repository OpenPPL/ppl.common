hpcc_populate_dep(pybind11)

find_package(Python3 COMPONENTS Development REQUIRED)
if(NOT Python3_Development_FOUND)
    message("python dev lib is required.")
endif()

file(GLOB_RECURSE PPLCOMMON_PYTHON_API_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
add_library(pypplcommon_shared SHARED ${PPLCOMMON_PYTHON_API_SRC})
target_link_libraries(pypplcommon_shared PUBLIC pplcommon_static)
target_include_directories(pypplcommon_shared PUBLIC
    ${pybind11_SOURCE_DIR}/include
    ${Python3_INCLUDE_DIRS})

# rename target for python import
set_target_properties(pypplcommon_shared PROPERTIES PREFIX "")
set_target_properties(pypplcommon_shared PROPERTIES OUTPUT_NAME "pypplcommon")
