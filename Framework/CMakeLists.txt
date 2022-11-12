cmake_minimum_required(VERSION 3.0.0)

message("PMC Framework build begin")
message("Target platform: " ${PMC_PLATFORM})

IF (PMC_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS                   "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS                 "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS          "-r -march=armv5t -mthumb" CACHE INTERNAL "")

add_compile_options(-r -march=armv5t -mthumb)

ELSEIF (PMC_PLATFORM STREQUAL "Win32")
message("Building for Win32")

ELSE ()
message(FATAL_ERROR "Invalid target platform!")

ENDIF ()

set(RPM_PLATFORM ${PMC_PLATFORM})
set(EXL_PLATFORM ${PMC_PLATFORM})

add_compile_definitions(RPM_STATIC_LIBRARY)

project(PMCFramework VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(-fno-rtti -fno-exceptions)

if (PMC_EXTLIB_PATH)
message("Using custom ExtLib " ${PMC_EXTLIB_PATH})
add_subdirectory(${CMAKE_SOURCE_DIR}/${PMC_EXTLIB_PATH} ${CMAKE_CURRENT_BINARY_DIR}/ExtLib)
else ()
message("Using builtin ExtLib")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/externals/ExtLib ${CMAKE_CURRENT_BINARY_DIR}/ExtLib)
endif ()

if (PMC_LIBRPM_PATH)
message("Using custom libRPM " ${PMC_LIBRPM_PATH})
add_subdirectory(${CMAKE_SOURCE_DIR}/${PMC_LIBRPM_PATH} ${CMAKE_CURRENT_BINARY_DIR}/libRPM)
else ()
message("Using builtin libRPM")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/externals/libRPM ${CMAKE_CURRENT_BINARY_DIR}/libRPM) 
endif ()

#Statically linking EXL should normally be discouraged, but we only need this part here

add_library(PMCFramework STATIC PMC_RPMFramework.cpp)
target_link_libraries(PMCFramework LibRPM.Static ExtLib.Heap)
if (PMC_SWAN_PATH)
message("Using custom swan " ${PMC_SWAN_PATH})
target_include_directories(PMCFramework PUBLIC . ${CMAKE_SOURCE_DIR}/${PMC_SWAN_PATH})
else ()
message("Using builtin swan")
target_include_directories(PMCFramework PUBLIC . externals/swan)
endif ()

add_library(PMCFramework.Include OBJECT PMC_Common.h)
target_include_directories(PMCFramework.Include PUBLIC .)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
