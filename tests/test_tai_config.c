#include "../include/tai_config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define PLUGIN "ur0:tai/rtsettings.suprx"
int main(void) {
    char out[2048], again[2048]; size_t n=0,n2=0; rttc_result r;
    const char *crlf="*KERNEL\r\nur0:tai/henkaku.skprx\r\n*NPXS10015\r\n# third-party comment\r\nur0:tai/other.suprx\r\n*main\r\n";
    r=rttc_add_settings_plugin(crlf,strlen(crlf),PLUGIN,out,sizeof(out),&n);
    assert(r==RTTC_ADDED);
    assert(strstr(out,"ur0:tai/other.suprx\r\n")!=NULL);
    assert(strstr(out,"ur0:tai/rtsettings.suprx\r\n*main")!=NULL);
    assert(strstr(out,"# third-party comment\r\n")!=NULL);
    r=rttc_add_settings_plugin(out,n,PLUGIN,again,sizeof(again),&n2);
    assert(r==RTTC_ALREADY_ACTIVE && n2==n && memcmp(out,again,n)==0);

    {
        const char *disabled="*NPXS10015\n# ur0:tai/rtsettings.suprx\n*main\n";
        r=rttc_add_settings_plugin(disabled,strlen(disabled),PLUGIN,out,sizeof(out),&n);
        assert(r==RTTC_DISABLED && n==strlen(disabled));
        r=rttc_enable_settings_plugin(disabled,strlen(disabled),PLUGIN,out,sizeof(out),&n);
        assert(r==RTTC_ENABLED && strstr(out,"\nur0:tai/rtsettings.suprx\n")!=NULL);
        assert(strstr(out,"# ur0:tai/rtsettings.suprx")==NULL);
        r=rttc_remove_settings_plugin(out,n,PLUGIN,again,sizeof(again),&n2);
        assert(r==RTTC_REMOVED && strstr(again,PLUGIN)==NULL);
        assert(strstr(again,"*NPXS10015\n")!=NULL && strstr(again,"*main\n")!=NULL);
    }

    {
        const char *missing="*KERNEL\nplugin.skprx";
        r=rttc_add_settings_plugin(missing,strlen(missing),PLUGIN,out,sizeof(out),&n);
        assert(r==RTTC_ADDED);
        assert(strstr(out,"*KERNEL\nplugin.skprx\n*NPXS10015\nur0:tai/rtsettings.suprx\n")!=NULL);
    }
    {
        const char *outside="*main\nur0:tai/rtsettings.suprx\n*NPXS10015\n";
        r=rttc_add_settings_plugin(outside,strlen(outside),PLUGIN,out,sizeof(out),&n);
        assert(r==RTTC_ADDED);
        assert(strstr(out,"*NPXS10015\nur0:tai/rtsettings.suprx\n")!=NULL);
    }
    {
        const char *duplicate="*NPXS10015\nur0:tai/rtsettings.suprx\n#ur0:tai/rtsettings.suprx\n";
        assert(rttc_add_settings_plugin(duplicate,strlen(duplicate),PLUGIN,out,sizeof(out),&n)==RTTC_AMBIGUOUS);
        assert(rttc_remove_settings_plugin(duplicate,strlen(duplicate),PLUGIN,out,sizeof(out),&n)==RTTC_AMBIGUOUS);
    }
    {
        const char *no_newline="*NPXS10015";
        r=rttc_add_settings_plugin(no_newline,strlen(no_newline),PLUGIN,out,sizeof(out),&n);
        assert(r==RTTC_ADDED && strstr(out,"*NPXS10015\nur0:tai/rtsettings.suprx\n")!=NULL);
    }
    assert(rttc_add_settings_plugin(NULL,0,PLUGIN,out,sizeof(out),&n)==RTTC_ERROR);
    assert(rttc_add_settings_plugin("*NPXS10015\n",11,PLUGIN,out,4,&n)==RTTC_CAPACITY);
    puts("taiHEN config preservation tests passed.");
    return 0;
}
