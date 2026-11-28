#ifndef RT_PLUGIN_INVENTORY_H
#define RT_PLUGIN_INVENTORY_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT_PI_MAX_ENTRIES 256
#define RT_PI_PATH_MAX 191
#define RT_PI_SECTION_MAX 63
#define RT_PI_PLUGIN_ID_MAX 63
#define RT_PI_ADAPTER_ID_MAX 95
#define RT_PI_MAX_ADAPTERS 64
typedef enum { RT_PI_OK=0, RT_PI_INVALID=-1, RT_PI_CAPACITY=-2, RT_PI_TEXT_TOO_LONG=-3, RT_PI_NOT_FOUND=-4, RT_PI_DUPLICATE=-5 } rt_pi_result;
typedef enum { RT_PI_ADAPTER_INVENTORY_ONLY=0, RT_PI_ADAPTER_CONFIGURABLE=1 } rt_pi_adapter_state;
typedef struct {
 char path[RT_PI_PATH_MAX+1], section[RT_PI_SECTION_MAX+1], plugin_id[RT_PI_PLUGIN_ID_MAX+1];
 unsigned int line_number, occurrence, duplicate_count;
 int enabled, known;
} rt_pi_entry;
typedef struct { rt_pi_entry entries[RT_PI_MAX_ENTRIES]; size_t count; size_t ignored_lines; size_t malformed_lines; } rt_pi_inventory;
typedef struct { char plugin_id[RT_PI_PLUGIN_ID_MAX+1], adapter_id[RT_PI_ADAPTER_ID_MAX+1]; } rt_pi_adapter;
typedef struct { rt_pi_adapter items[RT_PI_MAX_ADAPTERS]; size_t count; } rt_pi_adapter_registry;
void rt_pi_inventory_init(rt_pi_inventory *inventory);
int rt_pi_parse(const char *text, size_t length, rt_pi_inventory *inventory);
const rt_pi_entry *rt_pi_find_path(const rt_pi_inventory *inventory, const char *path, size_t occurrence);
void rt_pi_adapter_registry_init(rt_pi_adapter_registry *registry);
int rt_pi_adapter_register(rt_pi_adapter_registry *registry, const char *plugin_id, const char *adapter_id);
const rt_pi_adapter *rt_pi_adapter_find(const rt_pi_adapter_registry *registry, const char *plugin_id);
rt_pi_adapter_state rt_pi_entry_adapter_state(const rt_pi_entry *entry, const rt_pi_adapter_registry *registry);
#ifdef __cplusplus
}
#endif
#endif
