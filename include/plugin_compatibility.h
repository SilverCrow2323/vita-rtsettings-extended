#ifndef RT_PLUGIN_COMPATIBILITY_H
#define RT_PLUGIN_COMPATIBILITY_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT_PC_MAX_FINDINGS 64
#define RT_PC_MAX_RULES 64
#define RT_PC_PLUGIN_ID_MAX 63
typedef enum { RT_PC_OK=0, RT_PC_INVALID=-1, RT_PC_CAPACITY=-2 } rt_pc_result;
typedef enum { RT_PC_FINDING_MISSING_DEPENDENCY=1, RT_PC_FINDING_CONFLICT=2 } rt_pc_finding_type;
typedef enum { RT_PC_SEVERITY_BLOCK=1, RT_PC_SEVERITY_REVIEW=2 } rt_pc_severity;
typedef struct { rt_pc_finding_type type; rt_pc_severity severity; char plugin_id[RT_PC_PLUGIN_ID_MAX+1]; char related_plugin_id[RT_PC_PLUGIN_ID_MAX+1]; } rt_pc_finding;
typedef struct { rt_pc_finding_type type; rt_pc_severity severity; char plugin_id[RT_PC_PLUGIN_ID_MAX+1]; char related_plugin_id[RT_PC_PLUGIN_ID_MAX+1]; } rt_pc_rule;
typedef struct { rt_pc_rule rules[RT_PC_MAX_RULES]; size_t count; } rt_pc_rule_registry;
typedef struct { rt_pc_finding findings[RT_PC_MAX_FINDINGS]; size_t count, blocking_count, review_count; } rt_pc_report;
void rt_pc_rule_registry_init(rt_pc_rule_registry *registry);
int rt_pc_rule_register(rt_pc_rule_registry *registry, rt_pc_finding_type type, rt_pc_severity severity, const char *plugin_id, const char *related_plugin_id);
int rt_pc_evaluate_registry(const char *const *enabled_plugin_ids, size_t count, const rt_pc_rule_registry *registry, rt_pc_report *report);
int rt_pc_evaluate(const char *const *enabled_plugin_ids, size_t count, rt_pc_report *report);
const char *rt_pc_finding_type_name(rt_pc_finding_type type);
const char *rt_pc_severity_name(rt_pc_severity severity);
#ifdef __cplusplus
}
#endif
#endif
