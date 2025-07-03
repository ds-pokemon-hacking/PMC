cmake_minimum_required(VERSION 3.13.0)

message("Target platform: " ${PMC_PLATFORM})

IF (PMC_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "-r -march=armv5t -mthumb" CACHE INTERNAL "")

add_compile_options(-r -march=armv5t -mthumb)

ELSE ()
message(FATAL_ERROR "Invalid target platform!")

ENDIF ()

project(PMCInterface VERSION 0.1.0)
project(PMCInterface LANGUAGES C CXX ASM)

include(CTest)
enable_testing()

add_compile_options(-fno-rtti -fno-exceptions -fvisibility=hidden)

add_executable(PrintService.elf 
PrintService/PMC_Print.cpp
PrintService/PMC_Print.h
PrintService/PMC_ExceptionPrint.cpp
PrintService/PMC_ExceptionPrint.h
PrintService/PMC_PrintService.cpp
PrintService/PMC_PrintService.h
)
if (PMC_SWAN_PATH)
message("Using custom swan " ${PMC_SWAN_PATH})
target_include_directories(PrintService.elf PUBLIC . ${CMAKE_SOURCE_DIR}/${PMC_SWAN_PATH})
else ()
message("Using builtin swan")
target_include_directories(PrintService.elf PUBLIC . ../Framework/externals/swan)
endif ()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
