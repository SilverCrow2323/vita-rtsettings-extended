#include "tai_config.h"
#include <string.h>

#define SETTINGS_SECTION "*NPXS10015"

typedef struct {
    size_t start, content_end, line_end;
    size_t comment_at;
    int active, commented;
} line_match;

static int valid_plugin(const char *s) {
    size_t i;
    if (!s || !*s) return 0;
    for (i = 0; s[i]; ++i)
        if (s[i] == '\r' || s[i] == '\n' || s[i] == '#' || s[i] < 0x20) return 0;
    return 1;
}
static size_t line_content_end(const char *s, size_t start, size_t end) {
    if (end > start && s[end - 1] == '\r') return end - 1;
    return end;
}
static int is_header(const char *s, size_t start, size_t end) {
    size_t a = start, b = line_content_end(s, start, end), n = sizeof(SETTINGS_SECTION)-1;
    while (a < b && (s[a] == ' ' || s[a] == '\t')) ++a;
    while (b > a && (s[b-1] == ' ' || s[b-1] == '\t')) --b;
    return b-a == n && memcmp(s+a, SETTINGS_SECTION, n) == 0;
}
static int line_equals(const char *s, size_t start, size_t end, const char *value) {
    size_t a=start, b=line_content_end(s,start,end), n=strlen(value);
    while (a < b && (s[a]==' ' || s[a]=='\t')) ++a;
    while (b > a && (s[b-1]==' ' || s[b-1]=='\t')) --b;
    return b-a == n && memcmp(s+a,value,n)==0;
}
static int line_is_commented(const char *s, size_t start, size_t end,
                             const char *value, size_t *hash_position) {
    size_t a=start, b=line_content_end(s,start,end), n=strlen(value), hash;
    while (a < b && (s[a]==' ' || s[a]=='\t')) ++a;
    if (a >= b || s[a] != '#') return 0;
    hash=a++;
    while (a < b && (s[a]==' ' || s[a]=='\t')) ++a;
    while (b > a && (s[b-1]==' ' || s[b-1]=='\t')) --b;
    if (b-a != n || memcmp(s+a,value,n)!=0) return 0;
    if (hash_position) *hash_position=hash;
    return 1;
}
static int inspect_section(const char *s, size_t length, const char *plugin,
                           size_t *section_start, size_t *section_end,
                           line_match *match, size_t *matches) {
    size_t pos=0, current_start=0, current_end=length, count=0;
    int found=0;
    memset(match,0,sizeof(*match));
    while (pos < length) {
        size_t end=pos;
        while (end < length && s[end] != '\n') ++end;
        if (is_header(s,pos,end)) {
            if (found) { current_end=pos; break; }
            found=1; current_start=pos;
        } else if (found && s[pos]=='*') {
            current_end=pos; break;
        }
        pos = end < length ? end+1 : length;
    }
    if (!found) { *matches=0; return 0; }
    *section_start=current_start; *section_end=current_end;
    pos=current_start;
    while (pos < current_end) {
        size_t end=pos, hash=0;
        while (end < current_end && s[end] != '\n') ++end;
        if (line_equals(s,pos,end,plugin) ||
            line_is_commented(s,pos,end,plugin,&hash)) {
            if (count==0) {
                match->start=pos; match->content_end=line_content_end(s,pos,end);
                match->line_end=end < length ? end+1 : end;
                match->comment_at=hash;
                match->active=line_equals(s,pos,end,plugin);
                match->commented=!match->active;
            }
            ++count;
        }
        pos=end < current_end ? end+1 : current_end;
    }
    *matches=count;
    return 1;
}
static int choose_newline(const char *s, size_t length) {
    size_t i; for(i=0;i<length;++i) if(s[i]=='\n') return i>0 && s[i-1]=='\r';
    return 0;
}
static rttc_result copy_unchanged(const char *s,size_t n,char *out,size_t cap,size_t *outn,rttc_result result) {
    if (!out || !outn || n >= cap) return RTTC_CAPACITY;
    if (n) memcpy(out,s,n); out[n]='\0'; *outn=n; return result;
}
rttc_result rttc_add_settings_plugin(const char *s,size_t n,const char *plugin,
                                     char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0,insert_at,pl,extra; line_match m; int crlf;
    if(!s||!valid_plugin(plugin)||!out||!outn) return RTTC_ERROR;
    if(!inspect_section(s,n,plugin,&ss,&se,&m,&count))
        return RTTC_ERROR;
    if(count>1) return RTTC_AMBIGUOUS;
    if(count==1) return copy_unchanged(s,n,out,cap,outn,m.active?RTTC_ALREADY_ACTIVE:RTTC_DISABLED);
    crlf=choose_newline(s,n); pl=strlen(plugin); extra=pl+(crlf?2:1);
    if(ss==0 && se==n) { /* Section missing: append a new Settings section. */
        const char *section=SETTINGS_SECTION; size_t sl=sizeof(SETTINGS_SECTION)-1;
        size_t prefix=n && s[n-1]!='\n' ? (crlf?2:1) : 0;
        if(n+prefix+sl+(crlf?2:1)+extra+1>cap) return RTTC_CAPACITY;
        if(n) memcpy(out,s,n);
        insert_at=n;
        if(prefix){out[insert_at++]='\r';out[insert_at++]='\n';}
        memcpy(out+insert_at,section,sl);insert_at+=sl;
        if(crlf){out[insert_at++]='\r';out[insert_at++]='\n';}else out[insert_at++]='\n';
        memcpy(out+insert_at,plugin,pl);insert_at+=pl;
        if(crlf){out[insert_at++]='\r';out[insert_at++]='\n';}else out[insert_at++]='\n';
        out[insert_at]='\0';*outn=insert_at;return RTTC_ADDED;
    }
    insert_at=se;
    if(insert_at>0 && s[insert_at-1]!='\n') extra+=(crlf?2:1);
    if(n+extra+1>cap) return RTTC_CAPACITY;
    if(insert_at) memcpy(out,s,insert_at);
    {
        size_t p=insert_at;
        if(p>0 && s[p-1]!='\n') { if(crlf)out[p++]='\r'; out[p++]='\n'; }
        memcpy(out+p,plugin,pl);p+=pl;
        if(crlf){out[p++]='\r';out[p++]='\n';}else out[p++]='\n';
        memcpy(out+p,s+insert_at,n-insert_at);p+=n-insert_at;
        out[p]='\0';*outn=p;
    }
    (void)ss; return RTTC_ADDED;
}
rttc_result rttc_enable_settings_plugin(const char *s,size_t n,const char *plugin,
                                        char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0,pl,hash; line_match m;
    if(!s||!valid_plugin(plugin)||!out||!outn) return RTTC_ERROR;
    if(!inspect_section(s,n,plugin,&ss,&se,&m,&count)) return RTTC_NOT_FOUND;
    if(count>1) return RTTC_AMBIGUOUS;
    if(!count) return RTTC_NOT_FOUND;
    if(m.active) return copy_unchanged(s,n,out,cap,outn,RTTC_ALREADY_ACTIVE);
    hash=m.comment_at; pl=strlen(plugin);
    if(hash<m.start || hash>m.content_end || pl>m.content_end-hash-1) return RTTC_ERROR;
    if(n>=cap) return RTTC_CAPACITY;
    memcpy(out,s,m.start);
    memcpy(out+m.start,s+m.start,hash-m.start);
    memcpy(out+hash,plugin,pl);
    memcpy(out+hash+pl,s+m.content_end,n-m.content_end);
    *outn=hash+pl+(n-m.content_end);
    out[*outn]='\0';
    (void)ss;(void)se;return RTTC_ENABLED;
}
rttc_result rttc_remove_settings_plugin(const char *s,size_t n,const char *plugin,
                                        char *out,size_t cap,size_t *outn) {
    size_t ss=0,se=0,count=0; line_match m;
    if(!s||!valid_plugin(plugin)||!out||!outn) return RTTC_ERROR;
    if(!inspect_section(s,n,plugin,&ss,&se,&m,&count)) return RTTC_NOT_FOUND;
    if(count>1) return RTTC_AMBIGUOUS;
    if(!count) return copy_unchanged(s,n,out,cap,outn,RTTC_NOT_FOUND);
    if(n-(m.line_end-m.start)>=cap) return RTTC_CAPACITY;
    memcpy(out,s,m.start);
    memcpy(out+m.start,s+m.line_end,n-m.line_end);
    *outn=n-(m.line_end-m.start);out[*outn]='\0';
    (void)ss;(void)se;return RTTC_REMOVED;
}
