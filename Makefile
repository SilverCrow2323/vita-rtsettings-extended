CC ?= cc
AR ?= ar
RANLIB ?= ranlib
CPPFLAGS ?= -Iinclude -Isource/tai-saver -Isource/boot-monitor -Isource/preferences -Isource/layout-profiles -Isettings-ui -Isettings-ui/metadata -Isource/plugin-inventory -Isource/plugin-compatibility -Isource/plugin-adapters
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
SANITIZER_FLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

CORE_SOURCES := source/tai-saver/tai_saver.c source/tai-saver/sha256.c source/boot-monitor/boot_journal.c source/preferences/preferences.c source/layout-profiles/layout_profiles.c settings-ui/preferences_ui.c settings-ui/metadata/settings_metadata.c settings-ui/settings_screen.c settings-ui/profile_manager.c source/plugin-inventory/plugin_inventory.c source/plugin-compatibility/plugin_compatibility.c source/plugin-adapters/wifi_turbo_adapter.c
CORE_OBJECTS := build/tai_saver.o build/sha256.o build/boot_journal.o build/preferences.o build/layout_profiles.o build/preferences_ui.o build/settings_metadata.o build/settings_screen.o build/profile_manager.o build/plugin_inventory.o build/plugin_compatibility.o build/wifi_turbo_adapter.o
LIBRARY := build/librtsettings.a
TEST_BINARY := build/test_tai_saver
BOOT_TEST_BINARY := build/test_boot_journal
PREFERENCES_TEST_BINARY := build/test_preferences
PROFILE_TEST_BINARY := build/test_layout_profiles
UI_TEST_BINARY := build/test_preferences_ui
METADATA_TEST_BINARY := build/test_settings_metadata
PLUGIN_INVENTORY_TEST_BINARY := build/test_plugin_inventory
PLUGIN_COMPATIBILITY_TEST_BINARY := build/test_plugin_compatibility
WIFI_TURBO_TEST_BINARY := build/test_wifi_turbo_adapter
SCREEN_TEST_BINARY := build/test_settings_screen
PROFILE_MANAGER_TEST_BINARY := build/test_profile_manager
SANITIZER_BINARY := build/test_tai_saver_san
PLUGIN_SANITIZER_BINARY := build/test_plugin_inventory_san
PLUGIN_COMPATIBILITY_SANITIZER_BINARY := build/test_plugin_compatibility_san
WIFI_TURBO_SANITIZER_BINARY := build/test_wifi_turbo_adapter_san

.PHONY: all test sanitize clean
all: $(LIBRARY)

build:
	mkdir -p $@

build/tai_saver.o: source/tai-saver/tai_saver.c include/tai_saver.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/sha256.o: source/tai-saver/sha256.c source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/boot_journal.o: source/boot-monitor/boot_journal.c include/boot_journal.h include/tai_saver.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/preferences.o: source/preferences/preferences.c include/preferences.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/layout_profiles.o: source/layout-profiles/layout_profiles.c include/layout_profiles.h include/preferences.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/preferences_ui.o: settings-ui/preferences_ui.c settings-ui/preferences_ui.h include/preferences.h include/layout_profiles.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/settings_metadata.o: settings-ui/metadata/settings_metadata.c settings-ui/metadata/settings_metadata.h include/preferences.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/profile_manager.o: settings-ui/profile_manager.c settings-ui/profile_manager.h settings-ui/preferences_ui.h include/layout_profiles.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/settings_screen.o: settings-ui/settings_screen.c settings-ui/settings_screen.h settings-ui/preferences_ui.h settings-ui/metadata/settings_metadata.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/plugin_inventory.o: source/plugin-inventory/plugin_inventory.c include/plugin_inventory.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/plugin_compatibility.o: source/plugin-compatibility/plugin_compatibility.c include/plugin_compatibility.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/wifi_turbo_adapter.o: source/plugin-adapters/wifi_turbo_adapter.c include/wifi_turbo_adapter.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(CORE_OBJECTS)
	$(AR) rcs $@ $(CORE_OBJECTS)
	$(RANLIB) $@

$(TEST_BINARY): tests/test_tai_saver.c $(LIBRARY) include/tai_saver.h source/tai-saver/sha256.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(BOOT_TEST_BINARY): tests/test_boot_journal.c $(LIBRARY) include/boot_journal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PREFERENCES_TEST_BINARY): tests/test_preferences.c $(LIBRARY) include/preferences.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PROFILE_TEST_BINARY): tests/test_layout_profiles.c $(LIBRARY) include/layout_profiles.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(UI_TEST_BINARY): tests/test_preferences_ui.c $(LIBRARY) settings-ui/preferences_ui.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(METADATA_TEST_BINARY): tests/test_settings_metadata.c $(LIBRARY) settings-ui/metadata/settings_metadata.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PLUGIN_INVENTORY_TEST_BINARY): tests/test_plugin_inventory.c $(LIBRARY) include/plugin_inventory.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PLUGIN_COMPATIBILITY_TEST_BINARY): tests/test_plugin_compatibility.c $(LIBRARY) include/plugin_compatibility.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(WIFI_TURBO_TEST_BINARY): tests/test_wifi_turbo_adapter.c $(LIBRARY) include/wifi_turbo_adapter.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(SCREEN_TEST_BINARY): tests/test_settings_screen.c $(LIBRARY) settings-ui/settings_screen.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PROFILE_MANAGER_TEST_BINARY): tests/test_profile_manager.c $(LIBRARY) settings-ui/profile_manager.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

test: $(TEST_BINARY) $(BOOT_TEST_BINARY) $(PREFERENCES_TEST_BINARY) $(PROFILE_TEST_BINARY) $(UI_TEST_BINARY) $(METADATA_TEST_BINARY) $(PLUGIN_INVENTORY_TEST_BINARY) $(PLUGIN_COMPATIBILITY_TEST_BINARY) $(WIFI_TURBO_TEST_BINARY) $(SCREEN_TEST_BINARY) $(PROFILE_MANAGER_TEST_BINARY)
	./$(TEST_BINARY)
	./$(BOOT_TEST_BINARY)
	./$(PREFERENCES_TEST_BINARY)
	./$(PROFILE_TEST_BINARY)
	./$(UI_TEST_BINARY)
	./$(METADATA_TEST_BINARY)
	./$(PLUGIN_INVENTORY_TEST_BINARY)
	./$(PLUGIN_COMPATIBILITY_TEST_BINARY)
	./$(WIFI_TURBO_TEST_BINARY)
	./$(SCREEN_TEST_BINARY)
	./$(PROFILE_MANAGER_TEST_BINARY)

$(SANITIZER_BINARY): $(CORE_SOURCES) tests/test_tai_saver.c include/tai_saver.h source/tai-saver/sha256.h settings-ui/metadata/settings_metadata.h include/plugin_inventory.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_tai_saver.c -o $@ $(LDFLAGS)

$(PLUGIN_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_inventory.c include/plugin_inventory.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_inventory.c -o $@ $(LDFLAGS)

$(PLUGIN_COMPATIBILITY_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_compatibility.c include/plugin_compatibility.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_compatibility.c -o $@ $(LDFLAGS)

$(WIFI_TURBO_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_wifi_turbo_adapter.c include/wifi_turbo_adapter.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_wifi_turbo_adapter.c -o $@ $(LDFLAGS)

sanitize: $(SANITIZER_BINARY) $(PLUGIN_SANITIZER_BINARY) $(PLUGIN_COMPATIBILITY_SANITIZER_BINARY) $(WIFI_TURBO_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_COMPATIBILITY_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(WIFI_TURBO_SANITIZER_BINARY)

clean:
	rm -rf build
