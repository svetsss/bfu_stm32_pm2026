cmake_minimum_required(VERSION 3.18)
set(CMAKE_SYSTEM_NAME Generic)

# Disable compiler checking
# Otherwise it will fall with `reference to _exit() not found` error
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Select the cross compiler before project() enables the C language.
# ARM_TOOLCHAIN_PATH may name the bin directory, with or without a trailing slash.
file(TO_CMAKE_PATH "$ENV{ARM_TOOLCHAIN_PATH}" ARM_TOOLCHAIN_PATH)
find_program(ARM_GCC arm-none-eabi-gcc HINTS "${ARM_TOOLCHAIN_PATH}" REQUIRED)
set(CMAKE_C_COMPILER "${ARM_GCC}")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(starinskaya_lab4 C)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

set(TARGET_NAME ${PROJECT_NAME})

# Set compiler and utility path (actual if arm-none-eabi is not in PATH env variable)
get_filename_component(ARM_TOOLCHAIN_BIN "${ARM_GCC}" DIRECTORY)
find_program(OBJCOPY arm-none-eabi-objcopy HINTS "${ARM_TOOLCHAIN_BIN}" REQUIRED)
set(STFLASH st-flash)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb -std=c99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections -mlittle-endian -g3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-move-loop-invariants -fsigned-char -ffreestanding")

set(LINKER_FLAGS "-nostdlib -nostartfiles")
set(LINKER_FLAGS "${LINKER_FLAGS} -Xlinker -Map -Xlinker ${TARGET_NAME}.map ")
# Choose ld file for your MCU carefully
set(LINKER_FLAGS "${LINKER_FLAGS} -Xlinker -T ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103c8.ld")
# set(LINKER_FLAGS "${LINKER_FLAGS} -mcpu=cortex-m3 -mthumb -nostartfiles -g ") #Not necessary
set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,--gc-sections ")
# Passing flags to cmake linker variable
set(CMAKE_C_LINK_FLAGS ${LINKER_FLAGS})

message("DIRECTORY ${EXECUTABLE_OUTPUT_PATH}")

# Preprocessor definitions set MCU family and so-called "density"
add_definitions(-DSTM32F103C8)
add_definitions(-DSTM32F10X_MD)
# We don't use SPL in out template
# add_definitions(-DUSE_STDPERIPH_DRIVER)

include_directories(cmsis)
include_directories(stm32)

set(SOURCE_FILES
    stm32/startup/startup_stm32f10x_md.c # Choose startup file for your MCU
    stm32/system_stm32f10x.c
    syscalls/syscalls.c
    main.c)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Targets for make firmware in different formats
set(BIN_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.bin")
set(HEX_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.hex")
set(ELF_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")

add_custom_command(OUTPUT ${BIN_TARGET}
                    COMMAND ${OBJCOPY} -O binary ${ELF_TARGET} ${BIN_TARGET}
                    DEPENDS ${ELF_TARGET} ${PROJECT_NAME})

add_custom_target(bin DEPENDS ${BIN_TARGET})

add_custom_command(OUTPUT ${HEX_TARGET}
                    COMMAND ${OBJCOPY} -O ihex ${ELF_TARGET} ${HEX_TARGET}
                    DEPENDS ${ELF_TARGET} ${PROJECT_NAME})

add_custom_target(hex DEPENDS ${HEX_TARGET})

# Command and target to flash firmware
add_custom_command(OUTPUT flash_target
        COMMAND ${STFLASH} write ${BIN_TARGET} 0x8000000
        DEPENDS ${BIN_TARGET})

add_custom_target(flash DEPENDS flash_target)

# Команда для создания .bin файла из скомпилированного .elf
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating .bin file..."
)
