TARGET := rt-vita-cockpit
TITLE_ID := RTV000001
CC := arm-vita-eabi-gcc
CFLAGS := -O2 -Wall -Wextra -Wpedantic -std=c99 -fno-strict-aliasing
CPPFLAGS := -I$(VITASDK)/arm-vita-eabi/include -I../include -I../source/tai-saver -I../settings-ui -I../source/plugin-inventory -I../source/plugin-compatibility
LDFLAGS := -Wl,-q
LIBS := -lvita2d -lSceCtrl_stub -lSceIofilemgr_stub -lSceLibKernel_stub -lSceGxm_stub -lSceDisplay_stub -lScePgf_stub -lSceSysmodule_stub -lSceCommonDialog_stub -lm

TAI_SAVER_OBJECTS := tai_saver.o sha256.o rt_fs.o tai_config.o native_settings_installer.o plugin_inventory.o plugin_compatibility.o plugin_control_centre.o

all: native-settings-build $(TARGET).vpk

.PHONY: native-settings-build
native-settings-build:
	$(MAKE) -C ../native-settings clean all

../native-settings/rtsettings.suprx: native-settings-build

main.o: main.c ../include/tai_saver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

vita2d_user_app_compat.o: vita2d_user_app_compat.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tai_saver.o: ../source/tai-saver/tai_saver.c ../include/tai_saver.h ../include/rt_fs.h ../source/tai-saver/sha256.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

sha256.o: ../source/tai-saver/sha256.c ../source/tai-saver/sha256.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

rt_fs.o: ../source/platform/rt_fs.c ../include/rt_fs.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRTSE_PLATFORM_VITA -c $< -o $@

tai_config.o: ../source/tai-config/tai_config.c ../include/tai_config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

native_settings_installer.o: native_settings_installer.c native_settings_installer.h ../include/tai_config.h ../include/tai_saver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRTSE_PLATFORM_VITA -c $< -o $@

plugin_inventory.o: ../source/plugin-inventory/plugin_inventory.c ../include/plugin_inventory.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

plugin_compatibility.o: ../source/plugin-compatibility/plugin_compatibility.c ../include/plugin_compatibility.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

plugin_control_centre.o: ../settings-ui/plugin_control_centre.c ../settings-ui/plugin_control_centre.h ../include/plugin_inventory.h ../include/plugin_compatibility.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

eboot.elf: main.o vita2d_user_app_compat.o $(TAI_SAVER_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

eboot.bin: eboot.elf
	vita-make-fself -c $< $@

param.sfo:
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "Rt:Vita Cockpit" $@

$(TARGET).vpk: eboot.bin param.sfo ../native-settings/rtsettings.suprx
	vita-pack-vpk -s param.sfo -b eboot.bin -a ../native-settings/rtsettings.suprx=rtsettings.suprx $@

clean:
	rm -f *.o eboot.elf eboot.bin param.sfo $(TARGET).vpk
	$(MAKE) -C ../native-settings clean

.PHONY: all clean
