list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_X86)

file(GLOB_RECURSE PPLCOMMON_X86_SRC src/ppl/common/x86/*.cc)

if(NOT MSVC)
    list(APPEND PPLCOMMON_X86_SRC src/ppl/common/x86/cpuid_x86_64.S)
endif()

set_property(SOURCE src/ppl/common/half.cc PROPERTY COMPILE_FLAGS "-mf16c")
set_property(SOURCE src/ppl/common/x86/sysinfo.cc APPEND PROPERTY COMPILE_FLAGS "${FMA_ENABLED_FLAGS}")

list(APPEND PPLCOMMON_SRC ${PPLCOMMON_X86_SRC})

if (CMAKE_COMPILER_IS_GNUCC)
    list(APPEND PPLCOMMON_LINK_LIBRARIES pthread)
endif()

add_executable(cpuinfo tools/x86/cpuinfo.cc)

target_link_libraries(cpuinfo PRIVATE pplcommon_static)

# ----- installation ----- #

file(GLOB PPLCOMMON_X86_HEADERS
    src/ppl/common/x86/*.h)
install(FILES ${PPLCOMMON_X86_HEADERS}
    DESTINATION include/ppl/common/x86)
