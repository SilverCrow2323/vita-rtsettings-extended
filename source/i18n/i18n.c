#include "i18n.h"
#include <string.h>
static int copy_text(char *d,size_t c,const char *s){size_t n;if(!s)return 0;n=strlen(s);if(!n||n>=c)return 0;memcpy(d,s,n+1);return 1;}
static int valid_key(const char *s){size_t i;if(!s||!*s)return 0;for(i=0;s[i];++i){char c=s[i];if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='.'||c=='-'||c=='_'))return 0;}return 1;}
static int valid_locale(const char *s){size_t i,n;if(!s)return 0;n=strlen(s);if(n<2||n>RTP_I18N_LOCALE_MAX)return 0;for(i=0;i<n;++i){char c=s[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'))return 0;}return 1;}
void rtp_i18n_init(rtp_i18n_catalogue *c){if(c)memset(c,0,sizeof(*c));}
int rtp_i18n_register(rtp_i18n_catalogue *c,const rtp_i18n_descriptor *d){size_t i;rtp_i18n_entry e;
 if(!c||!d||!d->key||!d->locale||!d->text||!d->module_id||c->count>RTP_I18N_MAX_ENTRIES)return RTP_I18N_INVALID_ARGUMENT;
 if(!valid_key(d->key)||!valid_locale(d->locale)||!d->text[0]||!d->module_id[0])return RTP_I18N_INVALID_ARGUMENT;
 if(c->count>=RTP_I18N_MAX_ENTRIES)return RTP_I18N_CAPACITY;
 for(i=0;i<c->count;++i)if(!strcmp(c->entries[i].key,d->key)&&!strcmp(c->entries[i].locale,d->locale))return RTP_I18N_DUPLICATE;
 memset(&e,0,sizeof(e));if(!copy_text(e.key,sizeof(e.key),d->key)||!copy_text(e.locale,sizeof(e.locale),d->locale)||!copy_text(e.text,sizeof(e.text),d->text)||!copy_text(e.module_id,sizeof(e.module_id),d->module_id))return RTP_I18N_INVALID_ARGUMENT;
 c->entries[c->count++]=e;return RTP_I18N_OK;}
int rtp_i18n_remove_module(rtp_i18n_catalogue *c,const char *m){size_t i,out=0;if(!c||!m||!m[0]||c->count>RTP_I18N_MAX_ENTRIES)return RTP_I18N_INVALID_ARGUMENT;
 for(i=0;i<c->count;++i)if(strcmp(c->entries[i].module_id,m)){if(out!=i)c->entries[out]=c->entries[i];++out;}
 if(out==c->count)return RTP_I18N_NOT_FOUND;memset(&c->entries[out],0,(c->count-out)*sizeof(c->entries[0]));c->count=out;return RTP_I18N_OK;}
static const char *find_exact(const rtp_i18n_catalogue *c,const char *k,const char *l){size_t i;if(!l||!l[0])return NULL;for(i=0;i<c->count;++i)if(!strcmp(c->entries[i].key,k)&&!strcmp(c->entries[i].locale,l))return c->entries[i].text;return NULL;}
static void primary_locale(const char *l,char out[RTP_I18N_LOCALE_MAX+1]){size_t n=0;if(!l){out[0]=0;return;}while(l[n]&&l[n]!='-'&&n<RTP_I18N_LOCALE_MAX){out[n]=l[n];++n;}out[n]=0;}
const char *rtp_i18n_resolve(const rtp_i18n_catalogue *c,const char *k,const char *active,const char *fallback){char p[RTP_I18N_LOCALE_MAX+1];const char *s;
 if(!c||!k||!valid_key(k)||c->count>RTP_I18N_MAX_ENTRIES)return k;
 if((s=find_exact(c,k,active)))return s;primary_locale(active,p);if(p[0]&&(s=find_exact(c,k,p)))return s;
 /* The configured fallback is an exact locale, not a second language negotiation. */
 if((s=find_exact(c,k,fallback)))return s;return k;}
int rtp_i18n_has_locale(const rtp_i18n_catalogue *c,const char *l){size_t i;if(!c||!valid_locale(l)||c->count>RTP_I18N_MAX_ENTRIES)return 0;for(i=0;i<c->count;++i)if(!strcmp(c->entries[i].locale,l))return 1;return 0;}
