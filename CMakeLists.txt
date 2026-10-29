cmake_minimum_required(VERSION 3.20)

# This is a bare-metal Cortex-M project. Select the cross-compiler before
# project() so CMake never tries to use the macOS host compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(ARM_TOOLCHAIN_HINT "$ENV{ARM_TOOLCHAIN_PATH}")
find_program(ARM_GCC arm-none-eabi-gcc HINTS "${ARM_TOOLCHAIN_HINT}" REQUIRED)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy HINTS "${ARM_TOOLCHAIN_HINT}" REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size HINTS "${ARM_TOOLCHAIN_HINT}" REQUIRED)
set(CMAKE_C_COMPILER "${ARM_GCC}" CACHE FILEPATH "ARM bare-metal C compiler" FORCE)

project(stm32_template LANGUAGES C)

set(TARGET_NAME ${PROJECT_NAME})
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/stm32f103c8.ld")
set(BIN_FILE "${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin")
set(HEX_FILE "${CMAKE_BINARY_DIR}/${TARGET_NAME}.hex")
set(MAP_FILE "${CMAKE_BINARY_DIR}/${TARGET_NAME}.map")

add_executable(${TARGET_NAME}
    stm32/startup/startup_stm32f10x_md.c
    stm32/system_stm32f10x.c
    syscalls/syscalls.c
    main.c
)

set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_NAME}
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/cmsis"
    "${CMAKE_CURRENT_SOURCE_DIR}/stm32"
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    STM32F103C8
    STM32F10X_MD
)

target_compile_options(${TARGET_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -std=c99
    -ffunction-sections
    -fdata-sections
    -mlittle-endian
    -fsigned-char
    -ffreestanding
    -fno-move-loop-invariants
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

target_link_options(${TARGET_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -nostdlib
    -nostartfiles
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map,${MAP_FILE}
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> "${BIN_FILE}"
    COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> "${HEX_FILE}"
    COMMAND ${ARM_SIZE} $<TARGET_FILE:${TARGET_NAME}>
    BYPRODUCTS "${BIN_FILE}" "${HEX_FILE}"
    COMMENT "Generating BIN and HEX firmware images"
)

# Compatibility targets for the commands documented in README.md.
add_custom_target(bin DEPENDS ${TARGET_NAME})
add_custom_target(hex DEPENDS ${TARGET_NAME})

find_program(STFLASH st-flash)
if(STFLASH)
    add_custom_target(flash
        COMMAND ${STFLASH} write "${BIN_FILE}" 0x08000000
        DEPENDS ${TARGET_NAME}
        COMMENT "Flashing ${BIN_FILE} with ST-Link"
    )
endif()
