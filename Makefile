CC ?= cc
AR ?= ar
RANLIB ?= ranlib
CPPFLAGS ?= -Iinclude -Isource/tai-saver -Isource/boot-monitor -Isource/preferences -Isource/layout-profiles -Isettings-ui -Isettings-ui/metadata -Isettings-ui -Isource/plugin-inventory -Isource/plugin-compatibility -Isource/plugin-adapters -Isource/custom-assets -Isource/i18n
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
SANITIZER_FLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

CORE_SOURCES := source/tai-saver/tai_saver.c source/tai-saver/sha256.c source/boot-monitor/boot_journal.c source/preferences/preferences.c source/layout-profiles/layout_profiles.c settings-ui/preferences_ui.c settings-ui/metadata/settings_metadata.c settings-ui/settings_screen.c settings-ui/profile_manager.c settings-ui/module_controls.c settings-ui/service_manager.c settings-ui/plugin_control_centre.c settings-ui/plugin_help.c settings-ui/storage_devices.c source/plugin-inventory/plugin_inventory.c source/plugin-compatibility/plugin_compatibility.c source/plugin-adapters/wifi_turbo_adapter.c source/plugin-adapters/custom_warning_adapter.c source/custom-assets/custom_assets.c source/custom-assets/asset_manifest.c source/i18n/i18n.c source/platform/rt_fs.c source/tai-config/tai_config.c settings-ui/bluetooth_controllers.c settings-ui/repatch_manager.c settings-ui/adrenaline_config.c
CORE_OBJECTS := build/tai_saver.o build/sha256.o build/boot_journal.o build/preferences.o build/layout_profiles.o build/preferences_ui.o build/settings_metadata.o build/settings_screen.o build/profile_manager.o build/module_controls.o build/service_manager.o build/plugin_control_centre.o build/plugin_help.o build/storage_devices.o build/plugin_inventory.o build/plugin_compatibility.o build/wifi_turbo_adapter.o build/custom_warning_adapter.o build/custom_assets.o build/asset_manifest.o build/i18n.o build/rt_fs.o build/tai_config.o build/bluetooth_controllers.o build/repatch_manager.o build/adrenaline_config.o
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
PLUGIN_HELP_TEST_BINARY := build/test_plugin_help
STORAGE_TEST_BINARY := build/test_storage_devices
ASSET_TEST_BINARY := build/test_custom_assets
ASSET_MANIFEST_TEST_BINARY := build/test_asset_manifest
CUSTOM_WARNING_TEST_BINARY := build/test_custom_warning_adapter
BLUETOOTH_TEST_BINARY := build/test_bluetooth_controllers
REPATCH_TEST_BINARY := build/test_repatch_manager
ADRENALINE_TEST_BINARY := build/test_adrenaline_config
I18N_TEST_BINARY := build/test_i18n
VITA_ASSET_FS_TEST_BINARY := build/test_vita_asset_fs
VITA_ASSET_FS_SANITIZER_BINARY := build/test_vita_asset_fs_san
RT_FS_SANITIZER_BINARY := build/test_rt_fs_san
VITA_RT_FS_TEST_BINARY := build/test_rt_fs_vita
VITA_RT_FS_SANITIZER_BINARY := build/test_rt_fs_vita_san
SANITIZER_BINARY := build/test_tai_saver_san
PLUGIN_SANITIZER_BINARY := build/test_plugin_inventory_san
PLUGIN_COMPATIBILITY_SANITIZER_BINARY := build/test_plugin_compatibility_san
WIFI_TURBO_SANITIZER_BINARY := build/test_wifi_turbo_adapter_san
PROFILE_MANAGER_SANITIZER_BINARY := build/test_profile_manager_san
MODULE_CONTROLS_SANITIZER_BINARY := build/test_module_controls_san
SERVICE_MANAGER_SANITIZER_BINARY := build/test_service_manager_san
PLUGIN_CENTRE_SANITIZER_BINARY := build/test_plugin_control_centre_san
PLUGIN_HELP_SANITIZER_BINARY := build/test_plugin_help_san
STORAGE_SANITIZER_BINARY := build/test_storage_devices_san
ASSET_SANITIZER_BINARY := build/test_custom_assets_san
ASSET_MANIFEST_SANITIZER_BINARY := build/test_asset_manifest_san
CUSTOM_WARNING_SANITIZER_BINARY := build/test_custom_warning_adapter_san
BLUETOOTH_SANITIZER_BINARY := build/test_bluetooth_controllers_san
REPATCH_SANITIZER_BINARY := build/test_repatch_manager_san
ADRENALINE_SANITIZER_BINARY := build/test_adrenaline_config_san
NATIVE_SETTINGS_XML_TEST_BINARY := build/test_settings_xml
NATIVE_SETTINGS_XML_SANITIZER_BINARY := build/test_settings_xml_san
TAI_CONFIG_TEST_BINARY := build/test_tai_config
TAI_CONFIG_SANITIZER_BINARY := build/test_tai_config_san
NATIVE_RUNTIME_TEST_BINARY := build/test_settings_runtime
NATIVE_RUNTIME_SANITIZER_BINARY := build/test_settings_runtime_san

.PHONY: all test sanitize clean
all: $(LIBRARY)

build:
	mkdir -p $@

build/tai_saver.o: source/tai-saver/tai_saver.c include/tai_saver.h include/rt_fs.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/rt_fs.o: source/platform/rt_fs.c include/rt_fs.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/tai_config.o: source/tai-config/tai_config.c include/tai_config.h | build
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

build/plugin_help.o: settings-ui/plugin_help.c settings-ui/plugin_help.h settings-ui/plugin_control_centre.h include/preferences.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/storage_devices.o: settings-ui/storage_devices.c settings-ui/storage_devices.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/custom_assets.o: source/custom-assets/custom_assets.c include/custom_assets.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/asset_manifest.o: source/custom-assets/asset_manifest.c include/asset_manifest.h include/custom_assets.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/custom_warning_adapter.o: source/plugin-adapters/custom_warning_adapter.c include/custom_warning_adapter.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/i18n.o: source/i18n/i18n.c include/i18n.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/bluetooth_controllers.o: settings-ui/bluetooth_controllers.c settings-ui/bluetooth_controllers.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/repatch_manager.o: settings-ui/repatch_manager.c settings-ui/repatch_manager.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/adrenaline_config.o: settings-ui/adrenaline_config.c settings-ui/adrenaline_config.h | build
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

$(PLUGIN_HELP_TEST_BINARY): tests/test_plugin_help.c $(LIBRARY) settings-ui/plugin_help.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(ASSET_TEST_BINARY): tests/test_custom_assets.c $(LIBRARY) include/custom_assets.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(ASSET_MANIFEST_TEST_BINARY): tests/test_asset_manifest.c $(LIBRARY) include/asset_manifest.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(CUSTOM_WARNING_TEST_BINARY): tests/test_custom_warning_adapter.c $(LIBRARY) include/custom_warning_adapter.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(BLUETOOTH_TEST_BINARY): tests/test_bluetooth_controllers.c $(LIBRARY) settings-ui/bluetooth_controllers.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(REPATCH_TEST_BINARY): tests/test_repatch_manager.c $(LIBRARY) settings-ui/repatch_manager.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(ADRENALINE_TEST_BINARY): tests/test_adrenaline_config.c $(LIBRARY) settings-ui/adrenaline_config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(NATIVE_SETTINGS_XML_TEST_BINARY): tests/test_settings_xml.c native-settings/rt_settings_xml.c native-settings/rt_settings_xml.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_settings_xml.c native-settings/rt_settings_xml.c -o $@ $(LDFLAGS)

$(TAI_CONFIG_TEST_BINARY): tests/test_tai_config.c $(LIBRARY) include/tai_config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_tai_config.c $(LIBRARY) -o $@ $(LDFLAGS)

$(NATIVE_RUNTIME_TEST_BINARY): tests/test_settings_runtime.c native-settings/rt_settings_runtime.c native-settings/rt_settings_runtime.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_settings_runtime.c native-settings/rt_settings_runtime.c -o $@ $(LDFLAGS)

$(NATIVE_RUNTIME_SANITIZER_BINARY): tests/test_settings_runtime.c native-settings/rt_settings_runtime.c native-settings/rt_settings_runtime.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) tests/test_settings_runtime.c native-settings/rt_settings_runtime.c -o $@ $(LDFLAGS)

$(NATIVE_SETTINGS_XML_SANITIZER_BINARY): tests/test_settings_xml.c native-settings/rt_settings_xml.c native-settings/rt_settings_xml.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) tests/test_settings_xml.c native-settings/rt_settings_xml.c -o $@ $(LDFLAGS)

$(TAI_CONFIG_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_tai_config.c include/tai_config.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_tai_config.c -o $@ $(LDFLAGS)

$(I18N_TEST_BINARY): tests/test_i18n.c $(LIBRARY) include/i18n.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

$(VITA_ASSET_FS_TEST_BINARY): source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c include/vita_asset_fs.h include/custom_assets.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) $(CFLAGS) source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c -o $@ $(LDFLAGS)

$(VITA_RT_FS_TEST_BINARY): source/platform/rt_fs.c tests/test_rt_fs_vita.c include/rt_fs.h tests/vita-stubs/psp2/types.h tests/vita-stubs/psp2common/kernel/iofilemgr.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h tests/vita-stubs/psp2/io/dirent.h tests/vita-stubs/psp2/io/devctl.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) $(CFLAGS) source/platform/rt_fs.c tests/test_rt_fs_vita.c -o $@ $(LDFLAGS)

build/test_rt_fs: tests/test_rt_fs.c source/platform/rt_fs.c include/rt_fs.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_rt_fs.c source/platform/rt_fs.c -o $@ $(LDFLAGS)

test: $(PLUGIN_HELP_TEST_BINARY) $(CUSTOM_WARNING_TEST_BINARY) $(BLUETOOTH_TEST_BINARY) $(REPATCH_TEST_BINARY) $(ADRENALINE_TEST_BINARY) $(I18N_TEST_BINARY) $(ASSET_TEST_BINARY) $(ASSET_MANIFEST_TEST_BINARY) $(STORAGE_TEST_BINARY) $(PLUGIN_CENTRE_TEST_BINARY) $(TEST_BINARY) $(BOOT_TEST_BINARY) $(PREFERENCES_TEST_BINARY) $(PROFILE_TEST_BINARY) $(UI_TEST_BINARY) $(METADATA_TEST_BINARY) $(PLUGIN_INVENTORY_TEST_BINARY) $(PLUGIN_COMPATIBILITY_TEST_BINARY) $(WIFI_TURBO_TEST_BINARY) $(SCREEN_TEST_BINARY) $(PROFILE_MANAGER_TEST_BINARY) $(MODULE_CONTROLS_TEST_BINARY) $(SERVICE_MANAGER_TEST_BINARY) $(VITA_ASSET_FS_TEST_BINARY) build/test_rt_fs $(VITA_RT_FS_TEST_BINARY) $(NATIVE_SETTINGS_XML_TEST_BINARY) $(TAI_CONFIG_TEST_BINARY) $(NATIVE_RUNTIME_TEST_BINARY)
	./$(NATIVE_SETTINGS_XML_TEST_BINARY)
	./$(TAI_CONFIG_TEST_BINARY)
	./$(NATIVE_RUNTIME_TEST_BINARY)
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
	./$(PLUGIN_HELP_TEST_BINARY)
	./$(STORAGE_TEST_BINARY)
	./$(ASSET_TEST_BINARY)
	./$(ASSET_MANIFEST_TEST_BINARY)
	./$(CUSTOM_WARNING_TEST_BINARY)
	./$(BLUETOOTH_TEST_BINARY)
	./$(REPATCH_TEST_BINARY)
	./$(ADRENALINE_TEST_BINARY)
	./$(I18N_TEST_BINARY)
	./$(VITA_ASSET_FS_TEST_BINARY)
	./build/test_rt_fs
	./$(VITA_RT_FS_TEST_BINARY)

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

$(PLUGIN_HELP_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_help.c settings-ui/plugin_help.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_help.c -o $@ $(LDFLAGS)

$(PLUGIN_CENTRE_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_plugin_control_centre.c settings-ui/plugin_control_centre.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_plugin_control_centre.c -o $@ $(LDFLAGS)

$(STORAGE_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_storage_devices.c settings-ui/storage_devices.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_storage_devices.c -o $@ $(LDFLAGS)

$(SERVICE_MANAGER_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_service_manager.c settings-ui/service_manager.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_service_manager.c -o $@ $(LDFLAGS)

$(ASSET_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_custom_assets.c include/custom_assets.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_custom_assets.c -o $@ $(LDFLAGS)

$(ASSET_MANIFEST_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_asset_manifest.c include/asset_manifest.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_asset_manifest.c -o $@ $(LDFLAGS)

$(CUSTOM_WARNING_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_custom_warning_adapter.c include/custom_warning_adapter.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_custom_warning_adapter.c -o $@ $(LDFLAGS)

$(BLUETOOTH_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_bluetooth_controllers.c settings-ui/bluetooth_controllers.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_bluetooth_controllers.c -o $@ $(LDFLAGS)

$(REPATCH_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_repatch_manager.c settings-ui/repatch_manager.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_repatch_manager.c -o $@ $(LDFLAGS)

$(ADRENALINE_SANITIZER_BINARY): $(CORE_SOURCES) tests/test_adrenaline_config.c settings-ui/adrenaline_config.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_adrenaline_config.c -o $@ $(LDFLAGS)

$(VITA_ASSET_FS_SANITIZER_BINARY): source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c include/vita_asset_fs.h include/custom_assets.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) source/custom-assets/vita_asset_fs.c tests/test_vita_asset_fs.c -o $@ $(LDFLAGS)

$(RT_FS_SANITIZER_BINARY): tests/test_rt_fs.c source/platform/rt_fs.c include/rt_fs.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) tests/test_rt_fs.c source/platform/rt_fs.c -o $@ $(LDFLAGS)

$(VITA_RT_FS_SANITIZER_BINARY): source/platform/rt_fs.c tests/test_rt_fs_vita.c include/rt_fs.h tests/vita-stubs/psp2/types.h tests/vita-stubs/psp2common/kernel/iofilemgr.h tests/vita-stubs/psp2/io/fcntl.h tests/vita-stubs/psp2/io/stat.h tests/vita-stubs/psp2/io/dirent.h tests/vita-stubs/psp2/io/devctl.h | build
	$(CC) -DRTSE_PLATFORM_VITA -Itests/vita-stubs $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) source/platform/rt_fs.c tests/test_rt_fs_vita.c -o $@ $(LDFLAGS)

sanitize: $(VITA_RT_FS_SANITIZER_BINARY) $(RT_FS_SANITIZER_BINARY) $(PLUGIN_HELP_SANITIZER_BINARY) $(CUSTOM_WARNING_SANITIZER_BINARY) $(BLUETOOTH_SANITIZER_BINARY) $(REPATCH_SANITIZER_BINARY) $(ADRENALINE_SANITIZER_BINARY) $(ASSET_SANITIZER_BINARY) $(ASSET_MANIFEST_SANITIZER_BINARY) $(STORAGE_SANITIZER_BINARY) $(PLUGIN_CENTRE_SANITIZER_BINARY) $(SANITIZER_BINARY) $(PLUGIN_SANITIZER_BINARY) $(PLUGIN_COMPATIBILITY_SANITIZER_BINARY) $(WIFI_TURBO_SANITIZER_BINARY) $(PROFILE_MANAGER_SANITIZER_BINARY) $(MODULE_CONTROLS_SANITIZER_BINARY) $(SERVICE_MANAGER_SANITIZER_BINARY) $(VITA_ASSET_FS_SANITIZER_BINARY) $(NATIVE_SETTINGS_XML_SANITIZER_BINARY) $(TAI_CONFIG_SANITIZER_BINARY) $(NATIVE_RUNTIME_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(RT_FS_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(VITA_RT_FS_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(NATIVE_SETTINGS_XML_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(TAI_CONFIG_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(NATIVE_RUNTIME_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_COMPATIBILITY_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(WIFI_TURBO_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PROFILE_MANAGER_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(MODULE_CONTROLS_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SERVICE_MANAGER_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_CENTRE_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(PLUGIN_HELP_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(STORAGE_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(ASSET_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(ASSET_MANIFEST_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(CUSTOM_WARNING_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(BLUETOOTH_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(REPATCH_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(ADRENALINE_SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(VITA_ASSET_FS_SANITIZER_BINARY)

clean:
	rm -rf build
