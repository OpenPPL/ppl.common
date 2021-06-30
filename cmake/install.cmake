file(GLOB PPLCOMMON_HEADERS
    src/ppl/common/*.h)
install(FILES ${PPLCOMMON_HEADERS}
    DESTINATION include/ppl/common)

file(GLOB PPLCOMMON_PARAM_HEADERS
    src/ppl/common/params/*.h)
install(FILES ${PPLCOMMON_PARAM_HEADERS}
    DESTINATION include/ppl/common/params)

install(TARGETS pplcommon_static ARCHIVE DESTINATION lib)
