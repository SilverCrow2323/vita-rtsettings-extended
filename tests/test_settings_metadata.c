#include "settings_metadata.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 rtp_preferences p;rtp_metadata_catalogue c;rtp_metadata_descriptor d;const rtp_metadata_entry *e;
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
 assert(rtp_metadata_validate(&c,&p)==RTP_META_OK);
 puts("Settings metadata tests passed.");return 0;
}
