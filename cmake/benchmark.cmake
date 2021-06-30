hpcc_populate_dep(benchmark)

file(GLOB PPLCOMMON_BENCHMARK_SRC src/ppl/common/*_benchmark.cc)
add_executable(pplcommon_benchmark ${PPLCOMMON_BENCHMARK_SRC})
target_include_directories(pplcommon_benchmark PRIVATE ${benchmark_SOURCE_DIR}/include)
target_link_libraries(pplcommon_benchmark PRIVATE pplcommon_static benchmark_main)
