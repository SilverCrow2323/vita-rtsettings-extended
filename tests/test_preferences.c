#include "preferences.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define TEST_FILE "build/preferences-test.ini"
static void seed(rtp_preferences *p){
 rtp_preferences_init(p);
 assert(rtp_preferences_register(p,"advanced-options","",RTP_ENTRY_CATEGORY,1,20)==RTP_PREFS_OK);
 assert(rtp_preferences_register(p,"system-tools","",RTP_ENTRY_CATEGORY,1,30)==RTP_PREFS_OK);
 assert(rtp_preferences_register(p,"vita-shell","system-tools",RTP_ENTRY_TOOL,1,10)==RTP_PREFS_OK);
 assert(rtp_preferences_register(p,"ftp-auto-start","advanced-options",RTP_ENTRY_SETTING,1,20)==RTP_PREFS_OK);
}
static void test_visibility_reset(void){rtp_preferences p;const rtp_menu_entry *e;seed(&p);assert(rtp_preferences_set_visible(&p,"vita-shell",0)==RTP_PREFS_OK);e=rtp_preferences_find(&p,"vita-shell");assert(e&&e->visible==0);assert(rtp_preferences_reset_entry(&p,"vita-shell")==RTP_PREFS_OK);e=rtp_preferences_find(&p,"vita-shell");assert(e&&e->visible==1&&e->order==10);assert(rtp_preferences_set_visible(&p,"missing",0)==RTP_PREFS_NOT_FOUND);}
static void test_order_parent(void){rtp_preferences p;const rtp_menu_entry *e;seed(&p);assert(rtp_preferences_set_order(&p,"vita-shell",99)==RTP_PREFS_OK);assert(rtp_preferences_set_parent(&p,"vita-shell","advanced-options")==RTP_PREFS_OK);e=rtp_preferences_find(&p,"vita-shell");assert(e&&e->order==99&&strcmp(e->parent_id,"advanced-options")==0);assert(rtp_preferences_set_parent(&p,"advanced-options","vita-shell")==RTP_PREFS_INVALID_PARENT);assert(rtp_preferences_set_parent(&p,"advanced-options","system-tools")==RTP_PREFS_OK);assert(rtp_preferences_set_parent(&p,"system-tools","advanced-options")==RTP_PREFS_CYCLE);}
static void test_persistence(void){rtp_preferences p,q,old;const rtp_menu_entry *e;FILE *f;seed(&p);assert(rtp_preferences_set_visible(&p,"vita-shell",0)==RTP_PREFS_OK);assert(rtp_preferences_set_order(&p,"vita-shell",44)==RTP_PREFS_OK);assert(rtp_preferences_save(&p,TEST_FILE)==RTP_PREFS_OK);rtp_preferences_init(&q);assert(rtp_preferences_load(&q,TEST_FILE)==RTP_PREFS_OK);e=rtp_preferences_find(&q,"vita-shell");assert(e&&e->visible==0&&e->order==44);old=q;f=fopen(TEST_FILE,"wb");assert(f);fputs("BROKEN\n",f);fclose(f);assert(rtp_preferences_load(&q,TEST_FILE)==RTP_PREFS_INVALID_DATA);assert(memcmp(&q,&old,sizeof(q))==0);remove(TEST_FILE);}
static void test_invalid_registration(void){rtp_preferences p;seed(&p);assert(rtp_preferences_register(&p,"vita-shell","system-tools",RTP_ENTRY_TOOL,1,1)==RTP_PREFS_DUPLICATE_ID);assert(rtp_preferences_register(&p,"Bad ID","",RTP_ENTRY_ACTION,1,1)==RTP_PREFS_INVALID_ARGUMENT);assert(rtp_preferences_register(&p,"orphan-tool","missing-category",RTP_ENTRY_TOOL,1,1)==RTP_PREFS_INVALID_PARENT);}
int main(void){test_visibility_reset();test_order_parent();test_persistence();test_invalid_registration();puts("Preferences registry tests passed.");return 0;}
