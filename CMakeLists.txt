cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR AVR)

set(BINUTILS_PATH ${AVR_TOOLCHAIN_DIR})

set(TOOLCHAIN_PREFIX ${AVR_TOOLCHAIN_DIR}/bin/avr-)
set(TOOLCHAIN_INCLUDE ${AVR_TOOLCHAIN_DIR}/avr/include)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc.exe" CACHE FILEPATH "C Compiler path")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++.exe" CACHE FILEPATH "C++ Compiler path")

set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

set(CMAKE_C_STANDARD    11)
set(CMAKE_CXX_STANDARD  11)

# Disable compiler checks.
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(UOS_MCU 
  LANGUAGES ASM C CXX)

set(EXECUTABLE ${PROJECT_NAME}.elf)

# 해당 폴더의 파일만 찾는다.
file(GLOB SRC_FILES CONFIGURE_DEPENDS
  *.c
  *.cpp
)

# 해당 폴더를 포함해서 하위 폴더까지의 파일도 찾는다.
file(GLOB_RECURSE SRC_FILES_RECURSE CONFIGURE_DEPENDS
  src/*.c
  src/*.cpp
)

# Build the executable based on the source files
add_executable(${EXECUTABLE}  
  ${SRC_FILES}
  ${SRC_FILES_RECURSE}
  )

target_compile_definitions(${EXECUTABLE} PRIVATE
  -DF_CPU=1600000UL
  )

target_compile_definitions(${EXECUTABLE} PRIVATE
  -D__AVR_ATmega128__
  )

# List of include directories
target_include_directories(${EXECUTABLE} PRIVATE
  src 
  src/peripherals
  src/state_machines
  src/utils
  ${TOOLCHAIN_INCLUDE}
  )

target_include_directories(${EXECUTABLE} PRIVATE include)


# Compiler options
target_compile_options(${EXECUTABLE} PRIVATE
  -mmcu=avr128da48
  
  -fdata-sections
  -ffunction-sections
  -MMD
  -flto
  -fno-fat-lto-objects

  -Wall
  -Os
  -g3
  )

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
  -mmcu=avr128da48
  
  -flto 
  -fuse-linker-plugin

  -lm
  -Wl,-Map=${PROJECT_NAME}.map,--cref
  -Wl,--gc-sections
  -Xlinker -print-memory-usage -Xlinker
  )


add_custom_command(TARGET ${EXECUTABLE} 
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ARGS -O ihex -R .eeprom ${EXECUTABLE} ${PROJECT_NAME}.hex
  COMMENT "Invoking: Make Hex"
  )  

add_custom_command(TARGET ${EXECUTABLE} 
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ARGS -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 ${EXECUTABLE} ${PROJECT_NAME}.eep
  COMMENT "Invoking: Make EEPROM"
  )

