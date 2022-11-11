cmake_minimum_required(VERSION 3.10)

# Ensure we have CMAKE_C_FLAGS set...otherwise it will default to -Werror >.>
if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error -Wno-implicit-function-declaration")
endif()

set(EVMU_CORE_VERSION_MAJOR 2)
set(EVMU_CORE_VERSION_MINOR 0)
set(EVMU_CORE_VERSION_PATCH 0)
set(EVMU_CORE_VERSION
    ${EVMU_CORE_VERSION_MAJOR}.${EVMU_CORE_VERSION_MINOR}.${EVMU_CORE_VERSION_PATCH})

project(evmu-core VERSION ${EVMU_CORE_VERSION} DESCRIPTION "Lightweight, low-level emulator core library for the Dreamcast Visual Memory Unit" LANGUAGES C CXX)

add_subdirectory(lib)
add_subdirectory(legacy/libGyro)

option(EVMU_ENABLE_TESTS "Enable ElysianVmu unit tests" OFF)

if(EVMU_ENABLE_TESTS)
    add_subdirectory(test)
endif(EVMU_ENABLE_TESTS)

