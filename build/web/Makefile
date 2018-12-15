####### Compiler, tools and options

CC            = emcc
CXX           = emcc
DEFINES       = -DGYRO_PLATFORM=GYRO_PLATFORM_WEB
CFLAGS        = -pipe -msse -std=c99 -g -fPIC -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -msse -g -fPIC -std=gnu++1y -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I. -I../../libGyro/api -I../../libGyro/api/libGyro -I../../libGyro/include -I../../include/evmu-core -I/usr/lib/qt/mkspecs/linux-g++
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f
COMPRESS      = gzip -9f
AR            = ar cqs
RANLIB        =
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files

LIBGYRO_SOURCES       = ../../libGyro/source/common/gyro_system.c \
	../../libGyro/source/common/gyro_timer.c \
	../../libGyro/source/common/turfs_file_stdio.c \
	../../libGyro/source/common/gyro_file.c \
	../../libGyro/source/x86/turfs_unix.c \
	../../libGyro/source/common/gyro_visual_memory_common.c \
	../../libGyro/source/common/turfs_common.c \
	../../libGyro/source/web/gyro_debug_web.cpp \
	../../libGyro/source/web/gyro_audio_web.cpp \
	../../libGyro/source/web/gyro_timer_web.cpp \
	../../libGyro/source/web/gyro_file_web.cpp \
	../../libGyro/source/web/gyro_system_web.cpp \
	../../libGyro/source/web/gyro_net_web.cpp \
	../../libGyro/source/web/gyro_input_web.cpp

SOURCES       = ../../source/gyro_vmu_battery.c \
		../../source/gyro_vmu_bios.c \
		../../source/gyro_vmu_buzzer.c \
		../../source/gyro_vmu_cpu.c \
		../../source/gyro_vmu_device.c \
		../../source/gyro_vmu_disassembler.c \
		../../source/gyro_vmu_display.c \
		../../source/gyro_vmu_flash.c \
		../../source/gyro_vmu_gamepad.c \
		../../source/gyro_vmu_instr.c \
		../../source/gyro_vmu_lcd.c \
		../../source/gyro_vmu_maple.c \
		../../source/gyro_vmu_memory.c \
		../../source/gyro_vmu_osc.c \
		../../source/gyro_vmu_port1.c \
		../../source/gyro_vmu_port7.c \
		../../source/gyro_vmu_serial.c \
		../../source/gyro_vmu_tcp.c \
		../../source/gyro_vmu_timers.c \
		../../source/gyro_vmu_util.c \
		../../source/gyro_vmu_version.c \
		../../source/gyro_vmu_vmi.c \
		../../source/gyro_vmu_vms.c \
		../../source/gyro_vmu_isr.c \
		../../source/gyro_vmu_icondata.c \
		../../source/gyro_vmu_extra_bg_pvr.c \
		../../source/gyro_vmu_pso_img.c

OBJECTS       = gyro_vmu_battery.o \
		gyro_vmu_bios.o \
		gyro_vmu_buzzer.o \
		gyro_vmu_cpu.o \
		gyro_vmu_device.o \
		gyro_vmu_disassembler.o \
		gyro_vmu_display.o \
		gyro_vmu_flash.o \
		gyro_vmu_gamepad.o \
		gyro_vmu_instr.o \
		gyro_vmu_lcd.o \
		gyro_vmu_maple.o \
		gyro_vmu_memory.o \
		gyro_vmu_osc.o \
		gyro_vmu_port1.o \
		gyro_vmu_port7.o \
		gyro_vmu_serial.o \
		gyro_vmu_tcp.o \
		gyro_vmu_timers.o \
		gyro_vmu_util.o \
		gyro_vmu_version.o \
		gyro_vmu_vmi.o \
		gyro_vmu_vms.o \
		gyro_vmu_isr.o \
		gyro_vmu_icondata.o \
		gyro_vmu_extra_bg_pvr.o \
		gyro_vmu_pso_img.o

TARGET        = libevmu-core


first: all webapp
####### Build rules

staticlib: $(TARGET)

$(TARGET):  $(OBJECTS) $(OBJCOMP)

all: Makefile $(TARGET)

webapp:
	emcc -s FORCE_FILESYSTEM=1 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['FS']" -s EXIT_RUNTIME=1 -s TOTAL_MEMORY=256MB -w $(LIBGYRO_SOURCES) $(OBJECTS) $(wildcard source/*.cpp) --bind -o bin/evmu.js $(INCPATH)

clean: compiler_clean
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core

####### Sub-libraries

check: first

benchmark: first

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean:

####### Compile

gyro_vmu_battery.o: ../../source/gyro_vmu_battery.c ../../include/evmu-core/gyro_vmu_battery.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_battery.o ../../source/gyro_vmu_battery.c

gyro_vmu_bios.o: ../../source/gyro_vmu_bios.c ../../libGyro/api/libGyro/gyro_file_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_bios.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_bios.o ../../source/gyro_vmu_bios.c

gyro_vmu_buzzer.o: ../../source/gyro_vmu_buzzer.c ../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../libGyro/api/libGyro/gyro_matrix_api.h \
		../../libGyro/api/libGyro/gyro_math_api.h \
		../../libGyro/include/common/gyro_matrix_stack.h \
		../../libGyro/api/libGyro/gyro_audio_api.h \
		../../libGyro/api/libGyro/gyro_defines.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../include/evmu-core/gyro_vmu_isr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_buzzer.o ../../source/gyro_vmu_buzzer.c

gyro_vmu_cpu.o: ../../source/gyro_vmu_cpu.c ../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_osc.h \
		../../include/evmu-core/gyro_vmu_bios.h \
		../../include/evmu-core/gyro_vmu_util.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_disassembler.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_cpu.o ../../source/gyro_vmu_cpu.c

gyro_vmu_device.o: ../../source/gyro_vmu_device.c ../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_lcd.h \
		../../include/evmu-core/gyro_vmu_cpu.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_osc.h \
		../../libGyro/api/libGyro/gyro_file_api.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_device.o ../../source/gyro_vmu_device.c

gyro_vmu_disassembler.o: ../../source/gyro_vmu_disassembler.c ../../include/evmu-core/gyro_vmu_disassembler.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../libGyro/api/libGyro/gyro_file_api.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_disassembler.o ../../source/gyro_vmu_disassembler.c

gyro_vmu_display.o: ../../source/gyro_vmu_display.c ../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_display.o ../../source/gyro_vmu_display.c

gyro_vmu_flash.o: ../../source/gyro_vmu_flash.c ../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_vmi.h \
		../../include/evmu-core/gyro_vmu_lcd.h \
		../../include/evmu-core/gyro_vmu_util.h \
		../../include/evmu-core/gyro_vmu_extra_bg_pvr.h \
		../../include/evmu-core/gyro_vmu_icondata.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../libGyro/api/libGyro/gyro_defines.h \
		../../libGyro/api/libGyro/gyro_file_api.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_flash.o ../../source/gyro_vmu_flash.c

gyro_vmu_gamepad.o: ../../source/gyro_vmu_gamepad.c ../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../libGyro/api/libGyro/gyro_input_api.h \
		../../libGyro/api/libGyro/gyro_matrix_api.h \
		../../libGyro/api/libGyro/gyro_math_api.h \
		../../libGyro/include/common/gyro_matrix_stack.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_gamepad.o ../../source/gyro_vmu_gamepad.c

gyro_vmu_instr.o: ../../source/gyro_vmu_instr.c ../../include/evmu-core/gyro_vmu_instr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_instr.o ../../source/gyro_vmu_instr.c

gyro_vmu_lcd.o: ../../source/gyro_vmu_lcd.c ../../include/evmu-core/gyro_vmu_lcd.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../libGyro/api/libGyro/gyro_file_api.h \
		../../libGyro/api/libGyro/gyro_input_api.h \
		../../libGyro/api/libGyro/gyro_matrix_api.h \
		../../libGyro/api/libGyro/gyro_math_api.h \
		../../libGyro/include/common/gyro_matrix_stack.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_lcd.o ../../source/gyro_vmu_lcd.c

gyro_vmu_maple.o: ../../source/gyro_vmu_maple.c ../../include/evmu-core/gyro_vmu_maple.h \
		../../include/evmu-core/gyro_vmu_version.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_maple.o ../../source/gyro_vmu_maple.c

gyro_vmu_memory.o: ../../source/gyro_vmu_memory.c ../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_memory.o ../../source/gyro_vmu_memory.c

gyro_vmu_osc.o: ../../source/gyro_vmu_osc.c ../../include/evmu-core/gyro_vmu_osc.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_osc.o ../../source/gyro_vmu_osc.c

gyro_vmu_port1.o: ../../source/gyro_vmu_port1.c ../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_port1.o ../../source/gyro_vmu_port1.c

gyro_vmu_port7.o: ../../source/gyro_vmu_port7.c ../../include/evmu-core/gyro_vmu_port7.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_port7.o ../../source/gyro_vmu_port7.c

gyro_vmu_serial.o: ../../source/gyro_vmu_serial.c ../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_osc.h \
		../../include/evmu-core/gyro_vmu_cpu.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_serial.o ../../source/gyro_vmu_serial.c

gyro_vmu_tcp.o: ../../source/gyro_vmu_tcp.c ../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_maple.h \
		../../include/evmu-core/gyro_vmu_lcd.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_cpu.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_tcp.o ../../source/gyro_vmu_tcp.c

gyro_vmu_timers.o: ../../source/gyro_vmu_timers.c ../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_sfr.h \
		../../include/evmu-core/gyro_vmu_osc.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_timers.o ../../source/gyro_vmu_timers.c

gyro_vmu_util.o: ../../source/gyro_vmu_util.c ../../include/evmu-core/gyro_vmu_util.h \
		../../libGyro/api/libGyro/gyro_timer_api.h \
		../../libGyro/api/libGyro/gyro_platform.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_util.o ../../source/gyro_vmu_util.c

gyro_vmu_version.o: ../../source/gyro_vmu_version.c ../../include/evmu-core/gyro_vmu_version.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_version.o ../../source/gyro_vmu_version.c

gyro_vmu_vmi.o: ../../source/gyro_vmu_vmi.c ../../include/evmu-core/gyro_vmu_vmi.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_util.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_vmi.o ../../source/gyro_vmu_vmi.c

gyro_vmu_vms.o: ../../source/gyro_vmu_vms.c ../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_vms.o ../../source/gyro_vmu_vms.c

gyro_vmu_isr.o: ../../source/gyro_vmu_isr.c ../../include/evmu-core/gyro_vmu_isr.h \
		../../include/evmu-core/gyro_vmu_device.h \
		../../include/evmu-core/gyro_vmu_memory.h \
		../../include/evmu-core/gyro_vmu_instr.h \
		../../include/evmu-core/gyro_vmu_timers.h \
		../../include/evmu-core/gyro_vmu_buzzer.h \
		../../include/evmu-core/gyro_vmu_gamepad.h \
		../../include/evmu-core/gyro_vmu_display.h \
		../../include/evmu-core/gyro_vmu_serial.h \
		../../include/evmu-core/gyro_vmu_port1.h \
		../../include/evmu-core/gyro_vmu_flash.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../include/evmu-core/gyro_vmu_tcp.h \
		../../libGyro/api/libGyro/gyro_visual_memory_api.h \
		../../libGyro/api/libGyro/gyro_net_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../include/evmu-core/gyro_vmu_sfr.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_isr.o ../../source/gyro_vmu_isr.c

gyro_vmu_icondata.o: ../../source/gyro_vmu_icondata.c ../../include/evmu-core/gyro_vmu_icondata.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h \
		../../include/evmu-core/gyro_vmu_vms.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_icondata.o ../../source/gyro_vmu_icondata.c

gyro_vmu_extra_bg_pvr.o: ../../source/gyro_vmu_extra_bg_pvr.c ../../include/evmu-core/gyro_vmu_extra_bg_pvr.h \
		../../include/evmu-core/gyro_vmu_vms.h \
		../../libGyro/api/libGyro/gyro_system_api.h \
		../../libGyro/api/libGyro/gyro_platform.h \
		../../libGyro/include/common/gyro_system_common.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_extra_bg_pvr.o ../../source/gyro_vmu_extra_bg_pvr.c

gyro_vmu_pso_img.o: ../../source/gyro_vmu_pso_img.c ../../include/evmu-core/gyro_vmu_pso_img.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o gyro_vmu_pso_img.o ../../source/gyro_vmu_pso_img.c

####### Install

install:  FORCE

uninstall:  FORCE

FORCE:
