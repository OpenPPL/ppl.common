hpcc_populate_dep(googletest)

if(MSVC)
    hpcc_use_msvc_static_runtime()
endif()

file(GLOB PPLCOMMON_UNITTEST_SRC src/ppl/common/*_unittest.cc)
add_executable(pplcommon_unittest ${PPLCOMMON_UNITTEST_SRC})

target_link_libraries(pplcommon_unittest PRIVATE pplcommon_static gtest gtest_main)
target_include_directories(pplcommon_unittest PRIVATE ${googletest_SOURCE_DIR}/include)
