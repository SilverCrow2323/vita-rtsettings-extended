CC ?= cc
AR ?= ar
RANLIB ?= ranlib
CPPFLAGS ?= -Iinclude -Isource/tai-saver -Isource/boot-monitor -Isource/preferences -Isource/layout-profiles -Isettings-ui -Isettings-ui/metadata -Isource/plugin-inventory -Isource/plugin-compatibility -Isource/plugin-adapters -Isource/custom-assets
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
SANITIZER_FLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

CORE_SOURCES := source/tai-saver/tai_saver.c source/tai-saver/sha256.c source/boot-monitor/boot_journal.c source/preferences/preferences.c source/layout-profiles/layout_profiles.c settings-ui/preferences_ui.c settings-ui/metadata/settings_metadata.c settings-ui/settings_screen.c settings-ui/profile_manager.c settings-ui/module_controls.c settings-ui/service_manager.c settings-ui/plugin_control_centre.c settings-ui/storage_devices.c source/plugin-inventory/plugin_inventory.c source/plugin-compatibility/plugin_compatibility.c source/plugin-adapters/wifi_turbo_adapter.c source/plugin-adapters/custom_warning_adapter.c source/custom-assets/custom_assets.c
CORE_OBJECTS := build/tai_saver.o build/sha256.o build/boot_journal.o build/preferences.o build/layout_profiles.o build/preferences_ui.o build/settings_metadata.o build/settings_screen.o build/profile_manager.o build/module_controls.o build/service_manager.o build/plugin_control_centre.o build/storage_devices.o build/plugin_inventory.o build/plugin_compatibility.o build/wifi_turbo_adapter.o build/custom_warning_adapter.o build/custom_assets.o
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
MODULE_CONTROLS_TEST_BINARY := build/test_module_controls
SERVICE_MANAGER_TEST_BINARY := build/test_service_manager
PLUGIN_CENTRE_TEST_BINARY := build/test_plugin_control_centre
STORAGE_TEST_BINARY := build/test_storage_devices
ASSET_TEST_BINARY := build/test_custom_assets
CUSTOM_WARNING_TEST_BINARY := build/test_custom_warning_adapter
VITA_ASSET_FS_TEST_BINARY := build/test_vita_asset_fs
VITA_ASSET_FS_SANITIZER_BINARY := build/test_vita_asset_fs_san
SANITIZER_BINARY := build/test_tai_saver_san
PLUGIN_SANITIZER_BINARY := build/test_plugin_inventory_san
PLUGIN_COMPATIBILITY_SANITIZER_BINARY := build/test_plugin_compatibility_san
WIFI_TURBO_SANITIZER_BINARY := build/test_wifi_turbo_adapter_san
PROFILE_MANAGER_SANITIZER_BINARY := build/test_profile_manager_san
MODULE_CONTROLS_SANITIZER_BINARY := build/test_module_controls_san
SERVICE_MANAGER_SANITIZER_BINARY := build/test_service_manager_san
PLUGIN_CENTRE_SANITIZER_BINARY := build/test_plugin_control_centre_san
STORAGE_SANITIZER_BINARY := build/test_storage_devices_san
ASSET_SANITIZER_BINARY := build/test_custom_assets_san
CUSTOM_WARNING_SANITIZER_BINARY := build/test_custom_warning_adapter_san

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

build/service_manager.o: settings-ui/service_manager.c settings-ui/service_manager.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/module_controls.o: settings-ui/module_controls.c settings-ui/module_controls.h include/preferences.h | build
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

build/plugin_control_centre.o: settings-ui/plugin_control_centre.c settings-ui/plugin_control_centre.h include/plugin_inventory.h include/plugin_compatibility.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/storage_devices.o: settings-ui/storage_devices.c settings-ui/storage_devices.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/custom_assets.o: source/custom-assets/custom_assets.c include/custom_assets.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/custom_warning_adapter.o: source/plugin-adapters/custom_warning_adapter.c include/custom_warning_adapter.h | build
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

$(MODULE_CONTROLS_TEST_BINARY): tests/test_module_controls.c $(LIBRARY) settings-ui/module_controls.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(SERVICE_MANAGER_TEST_BINARY): tests/test_service_manager.c $(LIBRARY) settings-ui/service_manager.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(PLUGIN_CENTRE_TEST_BINARY): tests/test_plugin_control_centre.c $(LIBRARY) settings-ui/plugin_control_centre.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(STORAGE_TEST_BINARY): tests/test_storage_devices.c $(LIBRARY) settings-ui/storage_devices.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(ASSET_TEST_BINARY): tests/test_custom_assets.c $(LIBRARY) include/custom_assets.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(CUSTOM_WARNING_TEST_BINARY): tests/test_custom_warning_adapter.c $(LIBRARY) include/custom_warning_adapter.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(VITA_ASSET_FS_TEST_BINARY): source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c include/vita_asset_fs.h include/custom_assets.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) $(CFLAGS) source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c -o $@ $(LDFLAGS)

test: $(CUSTOM_WARNING_TEST_BINARY) $(ASSET_TEST_BINARY) $(STORAGE_TEST_BINARY) $(PLUGIN_CENTRE_TEST_BINARY) $(TEST_BINARY) $(BOOT_TEST_BINARY) $(PREFERENCES_TEST_BINARY) $(PROFILE_TEST_BINARY) $(UI_TEST_BINARY) $(METADATA_TEST_BINARY) $(PLUGIN_INVENTORY_TEST_BINARY) $(PLUGIN_COMPATIBILITY_TEST_BINARY) $(WIFI_TURBO_TEST_BINARY) $(SCREEN_TEST_BINARY) $(PROFILE_MANAGER_TEST_BINARY) $(MODULE_CONTROLS_TEST_BINARY) $(SERVICE_MANAGER_TEST_BINARY) $(VITA_ASSET_FS_TEST_BINARY)
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
	./$(MODULE_CONTROLS_TEST_BINARY)
	./$(SERVICE_MANAGER_TEST_BINARY)
	./$(PLUGIN_CENTRE_TEST_BINARY)
	./$(STORAGE_TEST_BINARY)
	./$(ASSET_TEST_BINARY)
	./$(CUSTOM_WARNING_TEST_BINARY)
	./$(VITA_ASSET_FS_TEST_BINARY)

$(SANITIZER_BINARY): $(CORE_SOURCES) tests/test_tai_saver.c include/tai_saver.h source/tai-saver/sha256.h settings-ui/metadata/settings_metadata.h include/plugin_inventory.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_tai_saver.c -o $@ $(LDFLAGS)

$(PLUGIN_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_inventory.c include/plugin_inventory.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_inventory.c -o $@ $(LDFLAGS)

$(PLUGIN_COMPATIBILITY_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_compatibility.c include/plugin_compatibility.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_compatibility.c -o $@ $(LDFLAGS)

$(WIFI_TURBO_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_wifi_turbo_adapter.c include/wifi_turbo_adapter.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_wifi_turbo_adapter.c -o $@ $(LDFLAGS)

$(PROFILE_MANAGER_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_profile_manager.c settings-ui/profile_manager.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_profile_manager.c -o $@ $(LDFLAGS)

$(MODULE_CONTROLS_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_module_controls.c settings-ui/module_controls.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_module_controls.c -o $@ $(LDFLAGS)

$(PLUGIN_CENTRE_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_control_centre.c settings-ui/plugin_control_centre.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_control_centre.c -o $@ $(LDFLAGS)

$(STORAGE_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_storage_devices.c settings-ui/storage_devices.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_storage_devices.c -o $@ $(LDFLAGS)

$(SERVICE_MANAGER_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_service_manager.c settings-ui/service_manager.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_service_manager.c -o $@ $(LDFLAGS)

$(ASSET_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_custom_assets.c include/custom_assets.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_custom_assets.c -o $@ $(LDFLAGS)

$(CUSTOM_WARNING_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_custom_warning_adapter.c include/custom_warning_adapter.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_custom_warning_adapter.c -o $@ $(LDFLAGS)

$(VITA_ASSET_FS_SANITIZER_BINARY): source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c include/vita_asset_fs.h include/custom_assets.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c -o $@ $(LDFLAGS)

sanitize: $(CUSTOM_WARNING_SANITIZER_BINARY) $(ASSET_SANITIZER_BINARY) $(STORAGE_SANITIZER_BINARY) $(PLUGIN_CENTRE_SANITIZER_BINARY) $(SANITIZER_BINARY) $(PLUGIN_SANITIZER_BINARY) $(PLUGIN_COMPATIBILITY_SANITIZER_BINARY) $(WIFI_TURBO_SANITIZER_BINARY) $(PROFILE_MANAGER_SANITIZER_BINARY) $(MODULE_CONTROLS_SANITIZER_BINARY) $(SERVICE_MANAGER_SANITIZER_BINARY) $(VITA_ASSET_FS_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_COMPATIBILITY_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(WIFI_TURBO_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PROFILE_MANAGER_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(MODULE_CONTROLS_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SERVICE_MANAGER_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_CENTRE_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(STORAGE_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(ASSET_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(CUSTOM_WARNING_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(VITA_ASSET_FS_SANITIZER_BINARY)

clean:
	rm -rf build
