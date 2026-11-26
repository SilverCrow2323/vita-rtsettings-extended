#include "i18n.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){rtp_i18n_catalogue c;rtp_i18n_descriptor d;rtp_i18n_init(&c);
 d.key="bluetooth.title";d.locale="en-US";d.text="Bluetooth";d.module_id="bluetooth";assert(rtp_i18n_register(&c,&d)==RTP_I18N_OK);
 d.locale="it-IT";d.text="Bluetooth";assert(rtp_i18n_register(&c,&d)==RTP_I18N_OK);
 d.key="bluetooth.input_learning";d.locale="en";d.text="Input learning";assert(rtp_i18n_register(&c,&d)==RTP_I18N_OK);
 assert(!strcmp(rtp_i18n_resolve(&c,"bluetooth.title","it-IT","en-US"),"Bluetooth"));
 assert(!strcmp(rtp_i18n_resolve(&c,"bluetooth.input_learning","en-GB","en-US"),"Input learning"));
 assert(!strcmp(rtp_i18n_resolve(&c,"bluetooth.input_learning","fr-FR","en-US"),"bluetooth.input_learning"));
 assert(rtp_i18n_has_locale(&c,"it-IT"));d.key="bluetooth.title";d.locale="it-IT";d.text="Duplicato";assert(rtp_i18n_register(&c,&d)==RTP_I18N_DUPLICATE);
 assert(rtp_i18n_remove_module(&c,"bluetooth")==RTP_I18N_OK);assert(!rtp_i18n_has_locale(&c,"it-IT"));
 assert(!strcmp(rtp_i18n_resolve(&c,"bluetooth.title","it-IT","en-US"),"bluetooth.title"));
 puts("Localization registry tests passed.");return 0;}