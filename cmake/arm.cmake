option(PPLCOMMON_USE_ARMV8_2 "build pplcommon with armv8.2-a support." OFF)
option(PPLCOMMON_USE_ARMV8_2_BF16 "build pplcommon with armv8.2-a+bf16 support. must enable PPLCOMMON_USE_ARMV8_2 first." OFF)
option(PPLCOMMON_USE_ARMV8_2_I8MM "build pplcommon with armv8.2-a+i8mm support. must enable PPLCOMMON_USE_ARMV8_2 first." OFF)

file(GLOB_RECURSE PPLCOMMON_ARM_SRC src/ppl/common/arm/*.cc)
list(APPEND PPLCOMMON_ARM_SRC src/ppl/common/arm/fp16fp32_cvt.S)

list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARM)

if (PPLCOMMON_USE_AARCH64)
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_AARCH64)
endif()

if (PPLCOMMON_USE_ARMV7)
    set_property(SOURCE src/ppl/common/half.cc PROPERTY COMPILE_FLAGS "-mfpu=neon-fp16")
    set_property(SOURCE src/ppl/common/arm/sysinfo.cc APPEND PROPERTY COMPILE_FLAGS " -mfpu=neon-fp16")
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARMV7)
endif()

if (PPLCOMMON_USE_ARMV8_2)
    set(ARMV8_2_MARCH "-march=armv8.2-a+fp16")
    if (PPLCOMMON_USE_ARMV8_2_BF16)
        set(ARMV8_2_MARCH "${ARMV8_2_MARCH}+bf16")
    endif()
    if (PPLCOMMON_USE_ARMV8_2_I8MM)
        set(ARMV8_2_MARCH "${ARMV8_2_MARCH}+i8mm")
    endif()
    set_property(SOURCE src/ppl/common/half.cc PROPERTY COMPILE_FLAGS ${ARMV8_2_MARCH})
    set_property(SOURCE src/ppl/common/arm/sysinfo.cc APPEND PROPERTY COMPILE_FLAGS ${ARMV8_2_MARCH})
endif()

if (PPLCOMMON_USE_ARMV8_2)
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARMV8_2)
    if (PPLCOMMON_USE_ARMV8_2_BF16)
        list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARMV8_2_BF16)
    endif()
    if (PPLCOMMON_USE_ARMV8_2_I8MM)
        list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_ARMV8_2_I8MM)
    endif()
endif()

list(APPEND PPLCOMMON_SRC ${PPLCOMMON_ARM_SRC})

# ----- installation ----- #

file(GLOB PPLCOMMON_ARM_HEADERS
    src/ppl/common/arm/*.h)
install(FILES ${PPLCOMMON_ARM_HEADERS}
    DESTINATION include/ppl/common/arm)
