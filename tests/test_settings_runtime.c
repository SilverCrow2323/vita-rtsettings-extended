#include "rt_settings_runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void check(const char *cfg,rt_settings_config_state expected,int module,int healthy) {
    rt_settings_runtime_snapshot s;
    assert(rt_settings_runtime_inspect(cfg,strlen(cfg),module,&s)==0);
    assert(s.config_state==expected);
    assert(s.healthy==healthy);
}
int main(void) {
    check("*NPXS10015\nur0:tai/rtsettings.suprx\n",RT_SETTINGS_CONFIG_ACTIVE,1,1);
    check("*NPXS10015\r\n# ur0:tai/rtsettings.suprx\r\n",RT_SETTINGS_CONFIG_DISABLED,1,0);
    check("*KERNEL\nur0:tai/rtsettings.suprx\n",RT_SETTINGS_CONFIG_SECTION_MISSING,1,0);
    check("*NPXS10015\n*main\n*NPXS10015\nur0:tai/rtsettings.suprx\n",RT_SETTINGS_CONFIG_DUPLICATE_SECTION,1,0);
    check("*NPXS10015\nur0:tai/rtsettings.suprx\n# ur0:tai/rtsettings.suprx\n",RT_SETTINGS_CONFIG_DUPLICATE_ENTRY,1,0);
    check("*NPXS10015\nur0:tai/other.suprx\n",RT_SETTINGS_CONFIG_ENTRY_MISSING,1,0);
    check("*NPXS10015\nur0:tai/rtsettings.suprx\n",RT_SETTINGS_CONFIG_ACTIVE,0,0);
    {
        rt_settings_runtime_snapshot s;
        assert(rt_settings_runtime_inspect(NULL,0,0,&s)<0);
        assert(rt_settings_runtime_inspect("*NPXS10015\n",11,2,&s)<0);
    }
    puts("Native Settings runtime inspection tests passed.");
    return 0;
}
