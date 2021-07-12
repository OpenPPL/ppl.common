if (NOT IS_X86)
    return()
endif()

list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_X86)
if(IS_X64)
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_X64)
endif()

file(GLOB_RECURSE PPLCOMMON_X86_SRC src/ppl/common/x86/*.cc)

if(NOT MSVC)
    if(IS_X64)
        list(APPEND PPLCOMMON_X86_SRC src/ppl/common/x86/cpuid_x86_64.S)
    else()
        list(APPEND PPLCOMMON_X86_SRC src/ppl/common/x86/cpuid_x86_32.S)
    endif()
endif()

set_property(SOURCE src/ppl/common/half.cc PROPERTY COMPILE_FLAGS "-mf16c")
set_property(SOURCE src/ppl/common/x86/sysinfo.cc APPEND PROPERTY COMPILE_FLAGS "${FMA_ENABLED_FLAGS}")

list(APPEND PPLCOMMON_SRC ${PPLCOMMON_X86_SRC})
list(APPEND PPLCOMMON_LINK_LIBRARIES pthread)

# ----- installation ----- #

file(GLOB PPLCOMMON_X86_HEADERS
    src/ppl/common/x86/*.h)
install(FILES ${PPLCOMMON_X86_HEADERS}
    DESTINATION include/ppl/common/x86)
