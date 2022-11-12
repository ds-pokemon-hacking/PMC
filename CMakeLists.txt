cmake_minimum_required(VERSION 3.0.0)

message("Target platform: " ${PMC_PLATFORM})

IF (PMC_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "-r -march=armv5t -mthumb" CACHE INTERNAL "")

add_compile_options(-r -march=armv5t -mthumb)
set(EXL_DESMUME 1)
add_compile_definitions(EXL_DESMUME)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_compile_definitions(DEBUG)
endif()
#add_compile_definitions(DEBUG)

ELSE ()
message(FATAL_ERROR "Invalid target platform!")

ENDIF ()

project(PMCInterface VERSION 0.1.0)
project(PMCInterface LANGUAGES C CXX ASM)

include(CTest)
enable_testing()

if (EXISTS "${CMAKE_SOURCE_DIR}/../extlib")
message("Found ExtLib in sibling directory!")
set(PMC_EXTLIB_PATH ../extlib)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/../libRPM")
message("Found libRPM in sibling directory!")
set(PMC_LIBRPM_PATH ../libRPM)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/../swan")
message("Found swan in sibling directory!")
set(PMC_SWAN_PATH ../swan)
endif()

add_subdirectory(Framework)

add_executable(PMC.elf 
Interface/PMC_AsmInterface.s 
Interface/PMC_Interface.h 
Interface/PMC_Interface.cpp 
Interface/PMC_ExceptionPrint.h
Interface/PMC_ExceptionPrint.cpp
Interface/PMC_Print.h
Interface/PMC_Print.cpp
)
target_link_libraries(PMC.elf PMCFramework)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
