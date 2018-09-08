#-------------------------------------------------
#
# Project created by QtCreator 2018-07-24T12:51:09
#
#-------------------------------------------------

QT       -= core gui

TARGET = evmu-core
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

isEmpty(LIBGYRO_PATH) {
    include(../../../../build/qtcreator/ElysianVMU/ElysianVMU.pri)
    LIBGYRO_PATH = ../../../libGyro
}

INCLUDEPATH += $${LIBGYRO_PATH}/api \
                $${LIBGYRO_PATH}/api/libGyro \
                $${LIBGYRO_PATH}/include \
                ../../../include/evmu-core

win32 {
    INCLUDEPATH += $${LIBGYRO_PATH}/lib/win32
}

DEPENDPATH += $${LIBGYRO_PATH}

SOURCES += \
    ../../../source/gyro_vmu_battery.c \
    ../../../source/gyro_vmu_bios.c \
    ../../../source/gyro_vmu_buzzer.c \
    ../../../source/gyro_vmu_cpu.c \
    ../../../source/gyro_vmu_device.c \
    ../../../source/gyro_vmu_disassembler.c \
    ../../../source/gyro_vmu_display.c \
    ../../../source/gyro_vmu_flash.c \
    ../../../source/gyro_vmu_gamepad.c \
    ../../../source/gyro_vmu_instr.c \
    ../../../source/gyro_vmu_lcd.c \
    ../../../source/gyro_vmu_maple.c \
    ../../../source/gyro_vmu_memory.c \
    ../../../source/gyro_vmu_osc.c \
    ../../../source/gyro_vmu_port1.c \
    ../../../source/gyro_vmu_port7.c \
    ../../../source/gyro_vmu_serial.c \
    ../../../source/gyro_vmu_tcp.c \
    ../../../source/gyro_vmu_timers.c \
    ../../../source/gyro_vmu_util.c \
    ../../../source/gyro_vmu_version.c \
    ../../../source/gyro_vmu_vmi.c \
    ../../../source/gyro_vmu_vms.c

HEADERS += \
    ../../../include/evmu-core/gyro_vmu_adapter.h \
    ../../../include/evmu-core/gyro_vmu_battery.h \
    ../../../include/evmu-core/gyro_vmu_bios.h \
    ../../../include/evmu-core/gyro_vmu_buzzer.h \
    ../../../include/evmu-core/gyro_vmu_cpu.h \
    ../../../include/evmu-core/gyro_vmu_device.h \
    ../../../include/evmu-core/gyro_vmu_disassembler.h \
    ../../../include/evmu-core/gyro_vmu_display.h \
    ../../../include/evmu-core/gyro_vmu_flash.h \
    ../../../include/evmu-core/gyro_vmu_gamepad.h \
    ../../../include/evmu-core/gyro_vmu_instr.h \
    ../../../include/evmu-core/gyro_vmu_isr.h \
    ../../../include/evmu-core/gyro_vmu_lcd.h \
    ../../../include/evmu-core/gyro_vmu_maple.h \
    ../../../include/evmu-core/gyro_vmu_memory.h \
    ../../../include/evmu-core/gyro_vmu_osc.h \
    ../../../include/evmu-core/gyro_vmu_port1.h \
    ../../../include/evmu-core/gyro_vmu_port7.h \
    ../../../include/evmu-core/gyro_vmu_serial.h \
    ../../../include/evmu-core/gyro_vmu_sfr.h \
    ../../../include/evmu-core/gyro_vmu_tcp.h \
    ../../../include/evmu-core/gyro_vmu_timers.h \
    ../../../include/evmu-core/gyro_vmu_util.h \
    ../../../include/evmu-core/gyro_vmu_version.h \
    ../../../include/evmu-core/gyro_vmu_vmi.h \
    ../../../include/evmu-core/gyro_vmu_vms.h

