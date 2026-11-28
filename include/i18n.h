#ifndef RTSETTINGS_I18N_H
#define RTSETTINGS_I18N_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_I18N_MAX_ENTRIES 512
#define RTP_I18N_KEY_MAX 95
#define RTP_I18N_LOCALE_MAX 15
#define RTP_I18N_TEXT_MAX 255
#define RTP_I18N_MODULE_MAX 47
typedef enum { RTP_I18N_OK=0, RTP_I18N_INVALID_ARGUMENT=-1, RTP_I18N_CAPACITY=-2, RTP_I18N_DUPLICATE=-3, RTP_I18N_NOT_FOUND=-4 } rtp_i18n_result;
typedef struct { const char *key; const char *locale; const char *text; const char *module_id; } rtp_i18n_descriptor;
typedef struct { char key[RTP_I18N_KEY_MAX+1]; char locale[RTP_I18N_LOCALE_MAX+1]; char text[RTP_I18N_TEXT_MAX+1]; char module_id[RTP_I18N_MODULE_MAX+1]; } rtp_i18n_entry;
typedef struct { rtp_i18n_entry entries[RTP_I18N_MAX_ENTRIES]; size_t count; } rtp_i18n_catalogue;
void rtp_i18n_init(rtp_i18n_catalogue *catalogue);
int rtp_i18n_register(rtp_i18n_catalogue *catalogue,const rtp_i18n_descriptor *descriptor);
int rtp_i18n_remove_module(rtp_i18n_catalogue *catalogue,const char *module_id);
const char *rtp_i18n_resolve(const rtp_i18n_catalogue *catalogue,const char *key,const char *active_locale,const char *fallback_locale);
int rtp_i18n_has_locale(const rtp_i18n_catalogue *catalogue,const char *locale);
#ifdef __cplusplus
}
#endif
#endif
