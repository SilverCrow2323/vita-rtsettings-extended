#include "rt_settings_runtime.h"
#include <string.h>
#define SETTINGS_SECTION "*NPXS10015"
#define SETTINGS_PLUGIN RT_SETTINGS_RUNTIME_PATH
static int line_equal(const char *line,size_t length,const char *value) {
    size_t n=strlen(value);
    return length==n && memcmp(line,value,n)==0;
}
static void trim_line(const char **line,size_t *length) {
    while(*length && (**line==' ' || **line=='\t')) { ++*line; --*length; }
    while(*length && ((*line)[*length-1]==' ' || (*line)[*length-1]=='\t' ||
                      (*line)[*length-1]=='\r')) --*length;
}
int rt_settings_runtime_inspect(const char *config,size_t length,int module_present,
                                rt_settings_runtime_snapshot *out) {
    size_t pos=0;
    int in_settings=0;
    if(!config || !out || module_present<0 || module_present>1) return -1;
    memset(out,0,sizeof(*out));
    out->module_present=module_present;
    while(pos<length) {
        size_t start=pos,n;
        const char *line;
        int disabled=0;
        while(pos<length && config[pos]!='\n') ++pos;
        n=pos-start;
        if(pos<length) ++pos;
        line=config+start;
        trim_line(&line,&n);
        if(!n) continue;
        if(line[0]=='*') {
            if(line_equal(line,n,SETTINGS_SECTION)) {
                out->settings_section_count++;
                in_settings=1;
            } else {
                in_settings=0;
            }
            continue;
        }
        if(!in_settings) continue;
        if(line[0]=='#') {
            disabled=1;
            ++line; --n;
            trim_line(&line,&n);
        }
        if(line_equal(line,n,SETTINGS_PLUGIN)) {
            if(disabled) out->disabled_entry_count++;
            else out->active_entry_count++;
        }
    }
    if(out->settings_section_count==0)
        out->config_state=RT_SETTINGS_CONFIG_SECTION_MISSING;
    else if(out->settings_section_count>1)
        out->config_state=RT_SETTINGS_CONFIG_DUPLICATE_SECTION;
    else if(out->active_entry_count+out->disabled_entry_count>1)
        out->config_state=RT_SETTINGS_CONFIG_DUPLICATE_ENTRY;
    else if(out->active_entry_count==1)
        out->config_state=RT_SETTINGS_CONFIG_ACTIVE;
    else if(out->disabled_entry_count==1)
        out->config_state=RT_SETTINGS_CONFIG_DISABLED;
    else
        out->config_state=RT_SETTINGS_CONFIG_ENTRY_MISSING;
    out->healthy=(out->config_state==RT_SETTINGS_CONFIG_ACTIVE && module_present);
    return 0;
}
const char *rt_settings_config_state_name(rt_settings_config_state state) {
    switch(state) {
        case RT_SETTINGS_CONFIG_ACTIVE: return "active";
        case RT_SETTINGS_CONFIG_DISABLED: return "disabled";
        case RT_SETTINGS_CONFIG_SECTION_MISSING: return "section-missing";
        case RT_SETTINGS_CONFIG_DUPLICATE_SECTION: return "duplicate-section";
        case RT_SETTINGS_CONFIG_DUPLICATE_ENTRY: return "duplicate-entry";
        case RT_SETTINGS_CONFIG_ENTRY_MISSING: return "entry-missing";
        default: return "invalid";
    }
}
