#ifndef RT_SETTINGS_RUNTIME_H
#define RT_SETTINGS_RUNTIME_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT_SETTINGS_RUNTIME_PATH "ur0:tai/rtsettings.suprx"
typedef enum {
    RT_SETTINGS_CONFIG_ACTIVE = 0,
    RT_SETTINGS_CONFIG_DISABLED,
    RT_SETTINGS_CONFIG_SECTION_MISSING,
    RT_SETTINGS_CONFIG_DUPLICATE_SECTION,
    RT_SETTINGS_CONFIG_DUPLICATE_ENTRY,
    RT_SETTINGS_CONFIG_ENTRY_MISSING,
    RT_SETTINGS_CONFIG_INVALID
} rt_settings_config_state;
typedef struct {
    rt_settings_config_state config_state;
    unsigned int settings_section_count;
    unsigned int active_entry_count;
    unsigned int disabled_entry_count;
    int module_present;
    int healthy;
} rt_settings_runtime_snapshot;
/* Inspects caller-supplied taiHEN text. It never writes or repairs the config. */
int rt_settings_runtime_inspect(const char *config, size_t length, int module_present,
                                rt_settings_runtime_snapshot *snapshot);
const char *rt_settings_config_state_name(rt_settings_config_state state);
#ifdef __cplusplus
}
#endif
#endif
