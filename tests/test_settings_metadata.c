#include "settings_metadata.h"
#include "settings_screen.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 rtp_preferences p;rtp_preferences_ui ui;rtp_metadata_catalogue c;rtp_metadata_descriptor d;rtp_screen_page page;const rtp_metadata_entry *e;
 rtp_preferences_init(&p);rtp_metadata_init(&c);
 assert(rtp_preferences_register(&p,"connectivity","",RTP_ENTRY_CATEGORY,1,10)==RTP_PREFS_OK);
 assert(rtp_preferences_register(&p,"ftp-service","connectivity",RTP_ENTRY_SETTING,1,10)==RTP_PREFS_OK);
 memset(&d,0,sizeof(d));d.entry_id="ftp-service";d.title="FTP Server";d.description="Start and manage the supported FTP service.";d.category_id="connectivity";d.icon_key="network";d.module_id="connectivity";
 assert(rtp_metadata_register(&c,&p,&d)==RTP_META_OK);
 e=rtp_metadata_find(&c,"ftp-service");assert(e&&strcmp(e->title,"FTP Server")==0&&strcmp(e->icon_key,"network")==0);
 assert(rtp_metadata_register(&c,&p,&d)==RTP_META_DUPLICATE_ID);
 d.entry_id="missing";assert(rtp_metadata_register(&c,&p,&d)==RTP_META_NOT_FOUND);
 rtp_metadata_init(&c);d.entry_id="ftp-service";d.category_id="not-a-category";assert(rtp_metadata_register(&c,&p,&d)==RTP_META_INVALID_CATEGORY);
 d.category_id="connectivity";d.title="";assert(rtp_metadata_register(&c,&p,&d)==RTP_META_INVALID_TEXT);
 d.title="FTP Server";assert(rtp_metadata_register(&c,&p,&d)==RTP_META_OK);
 printf("metadata validation=%d\\n",rtp_metadata_validate(&c,&p));assert(rtp_metadata_validate(&c,&p)==RTP_META_OK);
 rtp_preferences_init(&p);rtp_metadata_init(&c);
 assert(rtp_metadata_register_appearance(&c,&p)==RTP_META_OK);
 assert(rtp_metadata_validate(&c,&p)==RTP_META_OK);
 e=rtp_metadata_find(&c,"custom-assets-manager");assert(e&&strcmp(e->title,"Custom Assets Manager")==0);
 assert((e->flags&RTP_META_FLAG_REQUIRES_BACKEND)!=0);
 e=rtp_metadata_find(&c,"boot-animation");assert(e&&(e->flags&RTP_META_FLAG_REQUIRES_REBOOT)!=0);
 e=rtp_metadata_find(&c,"quick-menu-widgets");assert(e&&strcmp(e->module_id,"quick-menu")==0);
 assert(rtp_preferences_find(&p,"appearance-customization")!=NULL);
 rtp_preferences_init(&p);rtp_metadata_init(&c);
 assert(rtp_metadata_register_tai_saver(&c,&p)==RTP_META_OK);
 assert(rtp_metadata_validate(&c,&p)==RTP_META_OK);
 e=rtp_metadata_find(&c,"tai-saver");assert(e&&rtp_preferences_find(&p,"tai-saver")->kind==RTP_ENTRY_CATEGORY);
 e=rtp_metadata_find(&c,"tai-saver-backup-now");assert(e&&strcmp(e->category_id,"tai-saver-protection")==0&&e->flags&RTP_META_FLAG_REQUIRES_BACKEND);
 e=rtp_metadata_find(&c,"tai-saver-last-result");assert(e&&(e->flags&RTP_META_FLAG_READ_ONLY));
 e=rtp_metadata_find(&c,"tai-saver-known-good");assert(e&&strstr(e->description,"Never auto-rotated")!=NULL);
 assert(rtp_preferences_find(&p,"tai-saver-recovery")->kind==RTP_ENTRY_CATEGORY);
 assert(rtp_preferences_find(&p,"tai-saver-restore")->kind==RTP_ENTRY_ACTION);
 assert(rtp_preferences_find(&p,"tai-saver")->parent_id[0]=='\0');
 rtp_preferences_ui_init(&ui);
 assert(rtp_settings_screen_build(&ui,&p,&c,&page)==RTP_SCREEN_OK);
 assert(page.row_count==1&&strcmp(page.rows[0].id,"tai-saver")==0);
 assert(rtp_preferences_ui_enter(&ui,&p,"tai-saver")==RTP_UI_OK);
 assert(rtp_settings_screen_build(&ui,&p,&c,&page)==RTP_SCREEN_OK);
 assert(page.row_count==4&&strcmp(page.rows[0].id,"tai-saver-protection")==0);
 assert(rtp_preferences_ui_enter(&ui,&p,"tai-saver-protection")==RTP_UI_OK);
 assert(rtp_settings_screen_build(&ui,&p,&c,&page)==RTP_SCREEN_OK);
 assert(page.row_count==3&&strcmp(page.rows[1].id,"tai-saver-backup-now")==0&&page.rows[1].kind==RTP_ENTRY_ACTION);
 assert(rtp_preferences_ui_back(&ui)==RTP_UI_OK);
 assert(rtp_preferences_ui_enter(&ui,&p,"tai-saver-archives")==RTP_UI_OK);
 assert(rtp_settings_screen_build(&ui,&p,&c,&page)==RTP_SCREEN_OK);
 assert(page.row_count==4&&strcmp(page.rows[1].id,"tai-saver-known-good")==0);
 puts("Settings metadata tests passed.");return 0;
}
