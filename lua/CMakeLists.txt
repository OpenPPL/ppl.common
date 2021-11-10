if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARIES)
    find_package(Lua QUIET)
    if (NOT Lua_FOUND)
        message(FATAL_ERROR "lua dev lib >= 5.4.0 is required. please install lua development files or specify `LUA_INCLUDE_DIR` and `LUA_LIBRARIES` manually.")
    endif()
endif()

hpcc_populate_dep(luacpp)

file(GLOB_RECURSE PPLCOMMON_LUA_API_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_library(luapplcommon_shared SHARED ${PPLCOMMON_LUA_API_SRC})
target_link_libraries(luapplcommon_shared PUBLIC pplcommon_static luacpp_static ${LUA_LIBRARIES})
target_include_directories(luapplcommon_shared PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${HPCC_DEPS_DIR}/luacpp)
target_compile_features(luapplcommon_shared PRIVATE cxx_std_14)

# rename target for lua import
set_target_properties(luapplcommon_shared PROPERTIES PREFIX "")
set_target_properties(luapplcommon_shared PROPERTIES OUTPUT_NAME "common")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(luapplcommon_shared PROPERTIES SUFFIX ".so") # using .so instead of .dylib
endif()

install(TARGETS luapplcommon_shared DESTINATION lib/luappl)
