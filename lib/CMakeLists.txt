cmake_minimum_required(VERSION 3.10)

project(LibElysianVMU
        VERSION
            ${EVMU_VERSION}
        DESCRIPTION
            "Sega Dreamcast Visual Memory Unit Emulation Core Library"
        LANGUAGES
            C CXX)

set(EVMU_VERSION_MAJOR ${EVMU_VERSION_MAJOR} PARENT_SCOPE)
set(EVMU_VERSION_MINOR ${EVMU_VERSION_MINOR} PARENT_SCOPE)
set(EVMU_VERSION_PATCH ${EVMU_VERSION_PATCH} PARENT_SCOPE)
set(EVMU_VERSION ${EVMU_VERSION} PARENT_SCOPE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(EVMU_RESULT_ERROR_ASSERT             "Assert upon encountering API errors" ON)
option(EVMU_RESULT_ERROR_LOG                "Log API errors" ON)
option(EVMU_RESULT_CONTEXT_TRACK_LAST_ERROR "Track most recent error in EVMUContext" ON)
option(EVMU_RESULT_CALL_STACK_TRACKING      "Track calling source code location" ON)

set(EVMU_GIMBAL_CMAKE_PATH "lib/libgimbal" CACHE STRING "CMake Project Path for libGimbal API")
#set_property(ELYSIAN_LUA_CMAKE_PATH PROPERTY VALUE)

add_subdirectory(${EVMU_GIMBAL_CMAKE_PATH})

set(EVMU_SOURCES
    source/types/evmu_emulator.c
    source/types/evmu_ibehavior.c
    source/types/evmu_imemory.c
    source/hw/evmu_battery.c
    source/hw/evmu_buzzer.c
    source/hw/evmu_cpu.c
    source/hw/evmu_device.c
    source/types/evmu_peripheral.c
    source/fs/evmu_fat.c
    source/fs/evmu_file_manager.c
    source/fs/evmu_vmi.c
    source/fs/evmu_icondata.c
    source/fs/evmu_fs_utils.c
    source/fs/evmu_nexus.c
    source/fs/evmu_vms.c
    source/hw/evmu_gamepad.c
    source/hw/evmu_isa.c
    source/hw/evmu_pic.c
    source/hw/evmu_ram.c
    source/hw/evmu_clock.c
    source/hw/evmu_lcd.c
    source/hw/evmu_flash.c
    source/events/evmu_clock_event.c
    source/events/evmu_memory_event.c
    source/hw/evmu_rom.c
    source/hw/evmu_timers.c
    source/hw/evmu_wram.c
    )

set(EVMU_INCLUDES
    api/evmu/types/evmu_typedefs.h
    api/evmu/evmu_api.h
    api/evmu/types/evmu_emulator.h
    api/evmu/types/evmu_ibehavior.h
    api/evmu/types/evmu_peripheral.h
    api/evmu/types/evmu_imemory.h
    api/evmu/hw/evmu_wave.h
    api/evmu/hw/evmu_device.h
    api/evmu/hw/evmu_clock.h
    api/evmu/events/evmu_memory_event.h
    api/evmu/events/evmu_clock_event.h
    api/evmu/hw/evmu_rom.h
    api/evmu/hw/evmu_pic.h
    api/evmu/hw/evmu_flash.h
    api/evmu/hw/evmu_wram.h
    api/evmu/hw/evmu_address_space.h
    api/evmu/hw/evmu_isa.h
    api/evmu/hw/evmu_sfr.h
    api/evmu/hw/evmu_ram.h
    api/evmu/hw/evmu_battery.h
    api/evmu/hw/evmu_buzzer.h
    api/evmu/hw/evmu_lcd.h
    api/evmu/hw/evmu_gamepad.h
    api/evmu/hw/evmu_timers.h
    api/evmu/hw/evmu_cpu.h
    api/evmu/fs/evmu_fat.h
    api/evmu/fs/evmu_vmi.h
    api/evmu/fs/evmu_vms.h
    api/evmu/fs/evmu_icondata.h
    api/evmu/fs/evmu_file_manager.h
    api/evmu/fs/evmu_fs_utils.h
    api/evmu/fs/evmu_nexus.h
    source/hw/evmu_device_.h
    source/hw/evmu_ram_.h
    source/hw/evmu_cpu_.h
    source/hw/evmu_clock_.h
    source/hw/evmu_pic_.h
    source/hw/evmu_flash_.h
    source/hw/evmu_rom_.h
    source/hw/evmu_battery_.h
    source/types/evmu_peripheral_.h
    source/hw/evmu_buzzer_.h
    source/hw/evmu_lcd_.h
    source/hw/evmu_gamepad_.h
    source/hw/evmu_timers_.h
    source/hw/evmu_wram_.h
    source/fs/evmu_fat_.h
    source/types/evmu_marshal_.h
    source/types/evmu_marshal.c
    )

list(APPEND EVMU_SOURCES
    ../legacy/source/formats/gyro_vmu_extra_bg_pvr.c
    ../legacy/source/gyro_vmu_flash.c
    ../legacy/source/formats/gyro_vmu_icondata.c
    ../legacy/source/formats/gyro_vmu_pso_img.c
    ../legacy/source/formats/gyro_vmu_vmi.c
    ../legacy/source/formats/gyro_vmu_vms.c
    ../legacy/wrappers/cpp/source/evmu_device.cpp
    ../legacy/wrappers/cpp/source/evmu_flash.cpp)

list(APPEND EVMU_INCLUDES
    ../legacy/include/evmu-core/formats/gyro_vmu_extra_bg_pvr.h
    ../legacy/include/evmu-core/gyro_vmu_flash.h
    ../legacy/include/evmu-core/formats/gyro_vmu_icondata.h
    ../legacy/include/evmu-core/formats/gyro_vmu_lcd.h
    ../legacy/include/evmu-core/formats/gyro_vmu_pso_img.h
    ../legacy/include/evmu-core/formats/gyro_vmu_vmi.h
    ../legacy/include/evmu-core/formats/gyro_vmu_vms.h)

#if(FALSE)
#    api/evmu/hw/evmu_battery.h
#     api/evmu/hw/evmu_buzzer.h
#    api/evmu/hw/evmu_cpu.h

#    api/evmu/hw/evmu_display.h
#    api/evmu/hw/evmu_fat.h
#    api/evmu/hw/evmu_gamepad.h
#    api/evmu/hw/evmu_instr.h
#    api/evmu/hw/evmu_battery.h
#endif()

if(EVMU_RESULT_ERROR_ASSERT)
    list(APPEND
        EVMU_DEFINES
        EVMU_RESULT_ERROR_ASSERT)
endif()

if(EVMU_RESULT_ERROR_LOG)
    list(APPEND
        EVMU_DEFINES
        EVMU_RESULT_ERROR_LOG)
endif()

if(EVMU_ENABLE_TESTS)
    list(APPEND
        EVMU_DEFINES
        EVMU_ENABLE_TESTS)
endif()

if(EVMU_RESULT_CONTEXT_TRACK_LAST_ERROR)
    list(APPEND
        EVMU_DEFINES
        EVMU_RESULT_CONTEXT_TRACK_LAST_ERROR)
endif()

if(EVMU_RESULT_CALL_STACK_TRACKING)
    list(APPEND
        EVMU_DEFINES
        EVMU_RESULT_CALL_STACK_TRACKING)
endif()

add_library(libLibElysianVMU STATIC
    ${EVMU_SOURCES}
    ${EVMU_INCLUDES})

get_target_property(target_type libLibElysianVMU TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    list(APPEND
        EVMU_DEFINES
        GBL_CTX_SHARED_LIB)
endif()

target_include_directories(libLibElysianVMU
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
        $<INSTALL_INTERFACE:api>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../legacy/include>
        $<INSTALL_INTERFACE:../legacy/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../legacy/include/evmu-core>
        $<INSTALL_INTERFACE:../legacy/include/evmu-core>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../legacy/wrappers/cpp/include>
        $<INSTALL_INTERFACE:../legacy/wrappers/cpp/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/source #fixmelater
        ${CMAKE_CURRENT_SOURCE_DIR}/../legacy/include/evmu-core/formats #fixme
)

target_compile_definitions(libLibElysianVMU
    PUBLIC
    ${EVMU_DEFINES}
    -DEVMU_VERSION_MAJOR=${EVMU_VERSION_MAJOR}
    -DEVMU_VERSION_MINOR=${EVMU_VERSION_MINOR}
    -DEVMU_VERSION_PATCH=${EVMU_VERSION_PATCH}
    -DEVMU_VERSION="${EVMU_VERSION}")

target_link_libraries(libLibElysianVMU
    libGimbal)


