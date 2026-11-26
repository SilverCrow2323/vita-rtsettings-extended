#include "plugin_inventory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 const char cfg[]="# taiHEN config\r\n*KERNEL\r\nur0:tai/kubridge.skprx\r\nur0:tai/yamt_helper.skprx\r\n# ur0:tai/old_plugin.skprx\r\n*main\r\nur0:tai/ftpforvita.suprx\r\nur0:tai/ftpforvita.suprx\r\nnot-a-plugin\r\n[broken\r\n";
 rt_pi_inventory inv;rt_pi_adapter_registry reg;const rt_pi_entry *e;
 assert(rt_pi_parse(cfg,sizeof(cfg)-1,&inv)==RT_PI_OK);
 assert(inv.count==5);assert(inv.ignored_lines>=2);assert(inv.malformed_lines==1);
 e=rt_pi_find_path(&inv,"ur0:tai/kubridge.skprx",1);assert(e&&e->enabled&&e->known);assert(strcmp(e->plugin_id,"kubridge")==0);assert(strcmp(e->section,"*KERNEL")==0);
 e=rt_pi_find_path(&inv,"ur0:tai/old_plugin.skprx",1);assert(e&&!e->enabled&&!e->known);
 e=rt_pi_find_path(&inv,"ur0:tai/ftpforvita.suprx",2);assert(e&&e->duplicate_count==2&&e->occurrence==2);
 rt_pi_adapter_registry_init(&reg);assert(rt_pi_entry_adapter_state(rt_pi_find_path(&inv,"ur0:tai/ftpforvita.suprx",1),&reg)==RT_PI_ADAPTER_INVENTORY_ONLY);
 assert(rt_pi_adapter_register(&reg,"ftp-for-vita","connectivity.ftp_for_vita")==RT_PI_OK);
 assert(rt_pi_entry_adapter_state(rt_pi_find_path(&inv,"ur0:tai/ftpforvita.suprx",1),&reg)==RT_PI_ADAPTER_CONFIGURABLE);
 assert(rt_pi_adapter_register(&reg,"ftp-for-vita","duplicate")==RT_PI_DUPLICATE);
 assert(rt_pi_parse(NULL,0,&inv)==RT_PI_INVALID);
 puts("Plugin inventory tests passed.");return 0;
}
