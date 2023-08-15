file(GLOB PPLCOMMON_HEADERS
    src/ppl/common/*.h)
install(FILES ${PPLCOMMON_HEADERS}
    DESTINATION include/ppl/common)

file(GLOB PPLCOMMON_PARAM_HEADERS
    src/ppl/common/params/*.h)
install(FILES ${PPLCOMMON_PARAM_HEADERS}
    DESTINATION include/ppl/common/params)

if(MSVC)
    file(GLOB __win_headers__ src/ppl/common/windows/*.h)
    install(FILES ${__win_headers__} DESTINATION include/ppl/common/windows)
    unset(__win_headers__)
endif()

install(TARGETS pplcommon_static DESTINATION lib)

set(PPLCOMMON_CMAKE_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/pplcommon-config.cmake)
configure_file(cmake/pplcommon-config.cmake.in
    ${PPLCOMMON_CMAKE_CONFIG_FILE}
    @ONLY)
install(FILES ${PPLCOMMON_CMAKE_CONFIG_FILE} DESTINATION lib/cmake/ppl)
unset(PPLCOMMON_CMAKE_CONFIG_FILE)
