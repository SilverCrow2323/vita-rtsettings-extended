#include "plugin_help.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    rtp_plugin_help_catalogue c;
    rtp_plugin_help_panel p;
    rtp_plugin_help_descriptor d;
    rtp_plugin_help_init(&c);
    assert(rtp_plugin_help_register_builtin(&c)==RTP_HELP_OK);
    assert(c.count==2);
    assert(rtp_plugin_help_open(&c,"custom-warning",&p)==RTP_HELP_OK);
    assert(p.show_help_button && p.entry);
    assert(strcmp(p.entry->display_name,"Custom Warning (VitaTweaks)")==0);
    assert(strcmp(p.entry->author,"TheOfficialFloW")==0);
    assert(strstr(p.entry->technical_details,"UTF-16")!=NULL);
    assert(p.has_source_link && p.has_release_link);
    assert(rtp_plugin_help_open(&c,"unknown-plugin",&p)==RTP_HELP_NOT_FOUND);
    assert(!p.show_help_button && !p.entry);
    memset(&d,0,sizeof(d));
    d.plugin_id="custom-warning"; d.display_name="Duplicate"; d.author="Author";
    d.summary="Summary"; d.technical_details="Technical"; d.compatibility="Unknown";
    d.credits="Credits"; d.verification_note="Unverified";
    assert(rtp_plugin_help_register(&c,&d)==RTP_HELP_DUPLICATE);
    d.plugin_id="bad id";
    assert(rtp_plugin_help_register(&c,&d)==RTP_HELP_INVALID_ARGUMENT);
    d.plugin_id="valid-id"; d.display_name=""; 
    assert(rtp_plugin_help_register(&c,&d)==RTP_HELP_INVALID_TEXT);
    puts("Plugin help catalogue tests passed.");
    return 0;
}
