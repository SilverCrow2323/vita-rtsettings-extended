TARGET := rt-vita-cockpit
TITLE_ID := RTV000001
CC := arm-vita-eabi-gcc
CFLAGS := -O2 -Wall -Wextra -Wpedantic -std=c99
CPPFLAGS := -I$(VITASDK)/arm-vita-eabi/include
LDFLAGS := -Wl,-q
LIBS := -lvita2d -lSceCtrl_stub -lSceIofilemgr_stub -lSceLibKernel_stub -lSceGxm_stub -lSceDisplay_stub -lScePgf_stub -lm -lSceSysmodule_stub

all: $(TARGET).vpk

main.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

eboot.elf: main.o
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
