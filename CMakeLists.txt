cmake_minimum_required(VERSION 3.10)

set(EVMU_VERSION_MAJOR 1)
set(EVMU_VERSION_MINOR 5)
set(EVMU_VERSION_PATCH 1)
set(EVMU_VERSION
    ${EVMU_VERSION_MAJOR}.${EVMU_VERSION_MINOR}.${EVMU_VERSION_PATCH})

project(evmu-core VERSION ${EVMU_VERSION} DESCRIPTION "Lightweight, low-level emulator core library for the Dreamcast Visual Memory Unit" LANGUAGES C CXX)

add_subdirectory(lib)
add_subdirectory(legacy/libGyro)

option(EVMU_ENABLE_TESTS "Enable ElysianVmu unit tests" OFF)

if(EVMU_ENABLE_TESTS)
    add_subdirectory(test)
endif(EVMU_ENABLE_TESTS)

