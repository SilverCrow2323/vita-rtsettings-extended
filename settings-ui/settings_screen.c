#include "settings_screen.h"
#include <string.h>
static void copy(char *dst,size_t cap,const char *src){size_t n;if(!src)src="";n=strlen(src);if(n>=cap)n=cap-1;memcpy(dst,src,n);dst[n]='\\0';}
static void fallback_label(char *dst,size_t cap,const char *id){size_t i;int upper=1;copy(dst,cap,id);for(i=0;dst[i];++i){if(dst[i]=='-'||dst[i]=='_'||dst[i]=='.'){dst[i]=' ';upper=1;}else if(upper&&dst[i]>='a'&&dst[i]<='z'){dst[i]=(char)(dst[i]-'a'+'A');upper=0;}else upper=0;}}
int rtp_settings_screen_build(const rtp_preferences_ui *ui,const rtp_preferences *prefs,const rtp_metadata_catalogue *metadata,rtp_screen_page *page){
 rtp_ui_row rows[RTP_UI_MAX_ROWS];size_t count=0,i;const char *category;int rc,selected_found=0;
 if(!ui||!prefs||!metadata||!page||!ui->depth||ui->depth>RTP_UI_MAX_DEPTH)return RTP_SCREEN_INVALID_ARGUMENT;
 if(rtp_metadata_validate(metadata,prefs)!=RTP_META_OK)return RTP_SCREEN_MODEL_ERROR;
 memset(page,0,sizeof(*page));category=rtp_preferences_ui_current_category(ui);copy(page->category_id,sizeof(page->category_id),category);
 if(!category[0]){copy(page->title,sizeof(page->title),"Rt:Settings Extended");copy(page->description,sizeof(page->description),"System settings and installed extensions.");}
 else {const rtp_metadata_entry *m=rtp_metadata_find(metadata,category);if(m){copy(page->title,sizeof(page->title),m->title);copy(page->description,sizeof(page->description),m->description);}else fallback_label(page->title,sizeof(page->title),category);}
 rc=rtp_preferences_ui_rows(ui,prefs,rows,RTP_UI_MAX_ROWS,&count);if(rc!=RTP_UI_OK&&rc!=RTP_UI_EMPTY)return RTP_SCREEN_MODEL_ERROR;
 page->row_count=count;
 for(i=0;i<count;++i){const rtp_metadata_entry *m=rtp_metadata_find(metadata,rows[i].id);rtp_screen_row *out=&page->rows[i];copy(out->id,sizeof(out->id),rows[i].id);out->kind=rows[i].kind;if(m){copy(out->title,sizeof(out->title),m->title);copy(out->description,sizeof(out->description),m->description);copy(out->icon_key,sizeof(out->icon_key),m->icon_key);out->flags=m->flags;}else fallback_label(out->title,sizeof(out->title),rows[i].id);out->selected=(ui->frames[ui->depth-1].selected_id[0]&&strcmp(ui->frames[ui->depth-1].selected_id,rows[i].id)==0);if(out->selected)page->selected_index=i;}
 if(count&&!ui->frames[ui->depth-1].selected_id[0])page->rows[0].selected=1;
 return RTP_SCREEN_OK;
}
int rtp_settings_screen_render(const rtp_screen_page *page,const rtp_screen_renderer *r,void *ctx){size_t i;int failed=0;if(!page||!r||!r->begin_page||!r->draw_row||!r->draw_footer||!r->end_page)return RTP_SCREEN_INVALID_ARGUMENT;if(r->begin_page(ctx,page)!=0)return RTP_SCREEN_RENDER_ERROR;for(i=0;i<page->row_count;++i)if(r->draw_row(ctx,&page->rows[i],i,page->row_count)!=0){failed=1;break;}if(!failed&&r->draw_footer(ctx,"Select","Back")!=0)failed=1;if(r->end_page(ctx)!=0)failed=1;return failed?RTP_SCREEN_RENDER_ERROR:RTP_SCREEN_OK;}
