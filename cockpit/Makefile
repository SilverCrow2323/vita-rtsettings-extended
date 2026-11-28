TARGET := rt-vita-cockpit
TITLE_ID := RTV000001
CC := arm-vita-eabi-gcc
CFLAGS := -O2 -Wall -Wextra -Wpedantic -std=c99 -fno-strict-aliasing
CPPFLAGS := -I$(VITASDK)/arm-vita-eabi/include -I../include -I../source/tai-saver
LDFLAGS := -Wl,-q
LIBS := -lvita2d -lSceCtrl_stub -lSceIofilemgr_stub -lSceLibKernel_stub -lSceGxm_stub -lSceDisplay_stub -lScePgf_stub -lSceSysmodule_stub -lSceCommonDialog_stub -lm

TAI_SAVER_OBJECTS := tai_saver.o sha256.o rt_fs.o

all: $(TARGET).vpk

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

eboot.elf: main.o vita2d_user_app_compat.o $(TAI_SAVER_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

eboot.bin: eboot.elf
	vita-make-fself -c $< $@

param.sfo:
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "Rt:Vita Cockpit" $@

$(TARGET).vpk: eboot.bin param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $@

clean:
	rm -f *.o eboot.elf eboot.bin param.sfo $(TARGET).vpk

.PHONY: all clean
