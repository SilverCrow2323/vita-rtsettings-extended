#include "tai_config.h"
#include <string.h>

#define SETTINGS_SECTION "*NPXS10015"

typedef struct {
    size_t start, content_end, line_end, comment_at;
    int active, commented;
} line_match;

static int valid_plugin(const char *s) {
    size_t i;
    if (!s || !*s) return 0;
    for (i=0;s[i];++i)
        if (s[i]=='\r'||s[i]=='\n'||s[i]=='#'||s[i]<0x20) return 0;
    return 1;
}
static size_t content_end(const char *s,size_t start,size_t end) {
    return end>start && s[end-1]=='\r' ? end-1 : end;
}
static int is_target_header(const char *s,size_t start,size_t end) {
    size_t a=start,b=content_end(s,start,end),n=sizeof(SETTINGS_SECTION)-1;
    while(a<b&&(s[a]==' '||s[a]=='\t'))++a;
    while(b>a&&(s[b-1]==' '||s[b-1]=='\t'))--b;
    return b-a==n && memcmp(s+a,SETTINGS_SECTION,n)==0;
}
static int active_line(const char *s,size_t start,size_t end,const char *plugin) {
    size_t a=start,b=content_end(s,start,end),n=strlen(plugin);
    while(a<b&&(s[a]==' '||s[a]=='\t'))++a;
    while(b>a&&(s[b-1]==' '||s[b-1]=='\t'))--b;
    return b-a==n && memcmp(s+a,plugin,n)==0;
}
static int disabled_line(const char *s,size_t start,size_t end,const char *plugin,size_t *hash_out) {
    size_t a=start,b=content_end(s,start,end),n=strlen(plugin),hash;
    while(a<b&&(s[a]==' '||s[a]=='\t'))++a;
    if(a>=b||s[a]!='#')return 0;
    hash=a++;
    while(a<b&&(s[a]==' '||s[a]=='\t'))++a;
    while(b>a&&(s[b-1]==' '||s[b-1]=='\t'))--b;
    if(b-a!=n||memcmp(s+a,plugin,n)!=0)return 0;
    if(hash_out)*hash_out=hash;
    return 1;
}
/* Returns 1 for one unique section, 0 when absent, -1 for duplicate sections. */
static int inspect(const char *s,size_t length,const char *plugin,
                   size_t *section_start,size_t *section_end,
                   line_match *match,size_t *matches) {
    size_t pos=0,section_count=0,count=0,current_start=0,current_end=length;
    int in_target=0;
    memset(match,0,sizeof(*match));
    while(pos<length) {
        size_t end=pos,hash=0;
        while(end<length&&s[end]!='\n')++end;
        if(s[pos]=='*'||is_target_header(s,pos,end)) {
            if(in_target){current_end=pos;in_target=0;}
            if(is_target_header(s,pos,end)) {
                if(++section_count>1)return -1;
                current_start=pos;current_end=length;in_target=1;
            }
        } else if(in_target&&(active_line(s,pos,end,plugin)||disabled_line(s,pos,end,plugin,&hash))) {
            if(count==0) {
                match->start=pos;match->content_end=content_end(s,pos,end);
                match->line_end=end<length?end+1:end;match->comment_at=hash;
                match->active=active_line(s,pos,end,plugin);match->commented=!match->active;
            }
            ++count;
        }
        pos=end<length?end+1:length;
    }
    *matches=count;
    if(!section_count)return 0;
    *section_start=current_start;*section_end=current_end;
    return 1;
}
static int use_crlf(const char *s,size_t n) {
    size_t i;for(i=0;i<n;++i)if(s[i]=='\n')return i>0&&s[i-1]=='\r';
    return 0;
}
static rttc_result copy_same(const char *s,size_t n,char *out,size_t cap,size_t *outn,rttc_result status) {
    if(n>=cap)return RTTC_CAPACITY;
    if(n)memcpy(out,s,n);out[n]='\0';*outn=n;return status;
}
static rttc_result append_missing_section(const char *s,size_t n,const char *plugin,
                                          char *out,size_t cap,size_t *outn) {
    size_t p=n,pl=strlen(plugin),sl=sizeof(SETTINGS_SECTION)-1;
    int crlf=use_crlf(s,n);size_t nl=crlf?2:1;
    size_t prefix=n&&s[n-1]!='\n'?nl:0;
    if(n+prefix+sl+nl+pl+nl+1>cap)return RTTC_CAPACITY;
    if(n)memcpy(out,s,n);
    if(prefix){if(crlf)out[p++]='\r';out[p++]='\n';}
    memcpy(out+p,SETTINGS_SECTION,sl);p+=sl;
    if(crlf)out[p++]='\r';out[p++]='\n';
    memcpy(out+p,plugin,pl);p+=pl;
    if(crlf)out[p++]='\r';out[p++]='\n';
    out[p]='\0';*outn=p;return RTTC_ADDED;
}
rttc_result rttc_add_settings_plugin(const char *s,size_t n,const char *plugin,
                                     char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0,insert_at,pl,extra;line_match m;int crlf,found;
    if(!s||!valid_plugin(plugin)||!out||!outn)return RTTC_ERROR;
    found=inspect(s,n,plugin,&ss,&se,&m,&count);
    if(found<0)return RTTC_AMBIGUOUS;
    if(!found)return append_missing_section(s,n,plugin,out,cap,outn);
    if(count>1)return RTTC_AMBIGUOUS;
    if(count==1)return copy_same(s,n,out,cap,outn,m.active?RTTC_ALREADY_ACTIVE:RTTC_DISABLED);
    crlf=use_crlf(s,n);pl=strlen(plugin);extra=pl+(crlf?2:1);insert_at=se;
    if(insert_at>0&&s[insert_at-1]!='\n')extra+=(crlf?2:1);
    if(n+extra+1>cap)return RTTC_CAPACITY;
    if(insert_at)memcpy(out,s,insert_at);
    {
        size_t p=insert_at;
        if(p>0&&s[p-1]!='\n'){if(crlf)out[p++]='\r';out[p++]='\n';}
        memcpy(out+p,plugin,pl);p+=pl;
        if(crlf)out[p++]='\r';out[p++]='\n';
        memcpy(out+p,s+insert_at,n-insert_at);p+=n-insert_at;
        out[p]='\0';*outn=p;
    }
    (void)ss;return RTTC_ADDED;
}
rttc_result rttc_enable_settings_plugin(const char *s,size_t n,const char *plugin,
                                        char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0,pl,hash;line_match m;int found;
    if(!s||!valid_plugin(plugin)||!out||!outn)return RTTC_ERROR;
    found=inspect(s,n,plugin,&ss,&se,&m,&count);
    if(found<0||count>1)return RTTC_AMBIGUOUS;
    if(!found||!count)return RTTC_NOT_FOUND;
    if(m.active)return copy_same(s,n,out,cap,outn,RTTC_ALREADY_ACTIVE);
    hash=m.comment_at;pl=strlen(plugin);
    if(hash<m.start||hash>m.content_end||pl>m.content_end-hash-1)return RTTC_ERROR;
    if(n>=cap)return RTTC_CAPACITY;
    memcpy(out,s,m.start);
    memcpy(out+m.start,s+m.start,hash-m.start);
    memcpy(out+hash,plugin,pl);
    memcpy(out+hash+pl,s+m.content_end,n-m.content_end);
    *outn=hash+pl+(n-m.content_end);out[*outn]='\0';
    (void)ss;(void)se;return RTTC_ENABLED;
}
rttc_result rttc_remove_settings_plugin(const char *s,size_t n,const char *plugin,
                                        char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0;line_match m;int found;
    if(!s||!valid_plugin(plugin)||!out||!outn)return RTTC_ERROR;
    found=inspect(s,n,plugin,&ss,&se,&m,&count);
    if(found<0||count>1)return RTTC_AMBIGUOUS;
    if(!found||!count)return copy_same(s,n,out,cap,outn,RTTC_NOT_FOUND);
    if(n-(m.line_end-m.start)>=cap)return RTTC_CAPACITY;
    memcpy(out,s,m.start);memcpy(out+m.start,s+m.line_end,n-m.line_end);
    *outn=n-(m.line_end-m.start);out[*outn]='\0';
    (void)ss;(void)se;return RTTC_REMOVED;
}
