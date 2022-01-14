option(PPLCOMMON_USE_ARMV8_2 "build with arm v8.2 instruction set" OFF)

if (PPL_USE_ARMV8_2)
    set(PPLCOMMON_USE_ARMV8_2 ON)
endif()

file(GLOB_RECURSE PPLCOMMON_ARM_SRC src/ppl/common/arm/*.cc)
list(APPEND PPLCOMMON_ARM_SRC src/ppl/common/arm/fp16fp32_cvt.S)

list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARM)

if (PPLCOMMON_USE_ARMV8_2)
    set_property(SOURCE src/ppl/common/half.cc PROPERTY COMPILE_FLAGS "-march=armv8.2-a+fp16")
    set_property(SOURCE src/ppl/common/arm/sysinfo.cc APPEND PROPERTY COMPILE_FLAGS "-march=armv8.2-a+fp16")
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARMV8_2)
endif()

list(APPEND PPLCOMMON_SRC ${PPLCOMMON_ARM_SRC})

# ----- installation ----- #

file(GLOB PPLCOMMON_ARM_HEADERS
    src/ppl/common/arm/*.h)
install(FILES ${PPLCOMMON_ARM_HEADERS}
    DESTINATION include/ppl/common/arm)
