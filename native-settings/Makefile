VITASDK ?= $(shell echo $$VITASDK)
CC := arm-vita-eabi-gcc
ELF_CREATE := vita-elf-create
CFLAGS := -std=c99 -O2 -Wall -Wextra -Wpedantic -fno-strict-aliasing
CPPFLAGS := -I$(VITASDK)/arm-vita-eabi/include -I.
LDFLAGS := -Wl,-q -nostartfiles
TARGET := rtsettings
OBJECTS := rt_settings_main.o rt_settings_xml.o rt_settings_runtime.o

.PHONY: all clean
all: $(TARGET).suprx

rt_settings_main.o: rt_settings_main.c rt_settings_xml.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

rt_settings_xml.o: rt_settings_xml.c rt_settings_xml.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

rt_settings_runtime.o: rt_settings_runtime.c rt_settings_runtime.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -ltaihen_stub

$(TARGET).suprx: $(TARGET).elf rtsettings.yml
	$(ELF_CREATE) -e rtsettings.yml $< $@

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).suprx
