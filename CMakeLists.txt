cmake_minimum_required(VERSION 3.10)

set(EVMU_CORE_VERSION_MAJOR 2)
set(EVMU_CORE_VERSION_MINOR 0)
set(EVMU_CORE_VERSION_PATCH 0)
set(EVMU_CORE_VERSION
	${EVMU_CORE_VERSION_MAJOR}.${EVMU_CORE_VERSION_MINOR}.${EVMU_CORE_VERSION_PATCH})

project(evmu-core VERSION ${EVMU_CORE_VERSION} DESCRIPTION "Lightweight, low-level emulator core library for the Dreamcast Visual Memory Unit" LANGUAGES C CXX)


add_subdirectory(lib)
add_subdirectory(test)

