#include "preferences_ui.h"
#include <string.h>
static int is_ancestor_visible(const rtp_preferences *p,const rtp_menu_entry *e){size_t guard=0;while(e&&e->parent_id[0]){e=rtp_preferences_find(p,e->parent_id);if(!e||!e->visible)return 0;if(++guard>p->count)return 0;}return 1;}
static int row_before(const rtp_ui_row *a,const rtp_ui_row *b){if(a->order!=b->order)return a->order<b->order;return a->registry_index<b->registry_index;}
void rtp_preferences_ui_init(rtp_preferences_ui *ui){if(ui){memset(ui,0,sizeof(*ui));ui->depth=1;}}
const char *rtp_preferences_ui_current_category(const rtp_preferences_ui *ui){if(!ui||!ui->depth)return "";return ui->frames[ui->depth-1].category_id;}
int rtp_preferences_ui_enter(rtp_preferences_ui *ui,const rtp_preferences *p,const char *id){const rtp_menu_entry *e;if(!ui||!p||!id||ui->depth==0)return RTP_UI_INVALID_ARGUMENT;if(id[0]){e=rtp_preferences_find(p,id);if(!e||e->kind!=RTP_ENTRY_CATEGORY)return RTP_UI_NOT_FOUND;}if(ui->depth>=RTP_UI_MAX_DEPTH)return RTP_UI_CAPACITY;strcpy(ui->frames[ui->depth].category_id,id);ui->frames[ui->depth].selected_id[0]=0;ui->frames[ui->depth].selected_index=0;ui->depth++;return RTP_UI_OK;}
int rtp_preferences_ui_back(rtp_preferences_ui *ui){if(!ui||!ui->depth)return RTP_UI_INVALID_ARGUMENT;if(ui->depth<=1)return RTP_UI_ROOT;ui->depth--;return RTP_UI_OK;}
int rtp_preferences_ui_rows(const rtp_preferences_ui *ui,const rtp_preferences *p,rtp_ui_row *rows,size_t cap,size_t *count){
 size_t i,n=0;const char *parent;if(!ui||!p||!rows||!count||!ui->depth)return RTP_UI_INVALID_ARGUMENT;parent=rtp_preferences_ui_current_category(ui);
 for(i=0;i<p->count;i++){const rtp_menu_entry *e=&p->entries[i];rtp_ui_row row;size_t at;if(strcmp(e->parent_id,parent)||!e->visible||!is_ancestor_visible(p,e))continue;if(n>=cap)return RTP_UI_CAPACITY;memset(&row,0,sizeof(row));strcpy(row.id,e->id);row.kind=e->kind;row.order=e->order;row.registry_index=i;at=n;while(at>0&&row_before(&row,&rows[at-1])){rows[at]=rows[at-1];at--;}rows[at]=row;n++;}
 *count=n;return n?RTP_UI_OK:RTP_UI_EMPTY;
}
int rtp_preferences_ui_select(rtp_preferences_ui *ui,const rtp_preferences *p,const char *id){rtp_ui_row rows[RTP_UI_MAX_ROWS];size_t n,i;if(!ui||!p||!id)return RTP_UI_INVALID_ARGUMENT;if(rtp_preferences_ui_rows(ui,p,rows,RTP_UI_MAX_ROWS,&n)!=RTP_UI_OK)return RTP_UI_NOT_FOUND;for(i=0;i<n;i++)if(!strcmp(rows[i].id,id)){strcpy(ui->frames[ui->depth-1].selected_id,id);ui->frames[ui->depth-1].selected_index=i;return RTP_UI_OK;}return RTP_UI_NOT_FOUND;}
int rtp_preferences_ui_move(rtp_preferences_ui *ui,rtp_preferences *p,const char *id,int direction){
 rtp_ui_row rows[RTP_UI_MAX_ROWS];size_t n,i;unsigned int a,b;int rc;if(!ui||!p||!id||(direction!=-1&&direction!=1))return RTP_UI_INVALID_ARGUMENT;rc=rtp_preferences_ui_rows(ui,p,rows,RTP_UI_MAX_ROWS,&n);if(rc!=RTP_UI_OK)return rc;for(i=0;i<n;i++)if(!strcmp(rows[i].id,id))break;if(i==n)return RTP_UI_NOT_FOUND;if((direction<0&&i==0)||(direction>0&&i+1>=n))return RTP_UI_EMPTY;a=rows[i].order;b=rows[i+(direction>0?1:(size_t)-1)].order;
 /* Equal order values are still safely movable: assign adjacent unique positions. */
 if(a==b){size_t j;for(j=0;j<n;j++)if(rtp_preferences_set_order(p,rows[j].id,(unsigned int)(j*10+10))!=RTP_PREFS_OK)return RTP_UI_INVALID_ARGUMENT;rc=rtp_preferences_ui_rows(ui,p,rows,RTP_UI_MAX_ROWS,&n);if(rc!=RTP_UI_OK)return rc;for(i=0;i<n;i++)if(!strcmp(rows[i].id,id))break;a=rows[i].order;b=rows[i+(direction>0?1:(size_t)-1)].order;}
 if(rtp_preferences_set_order(p,id,b)!=RTP_PREFS_OK||rtp_preferences_set_order(p,rows[i+(direction>0?1:(size_t)-1)].id,a)!=RTP_PREFS_OK)return RTP_UI_INVALID_ARGUMENT;return RTP_UI_OK;
}
int rtp_preferences_ui_set_visible(rtp_preferences_ui *ui,rtp_preferences *p,const char *id,int visible){
 const rtp_menu_entry *e;int rc;if(!ui||!p||!id)return RTP_UI_INVALID_ARGUMENT;rc=rtp_preferences_set_visible(p,id,visible);if(rc!=RTP_PREFS_OK)return RTP_UI_NOT_FOUND;
 e=rtp_preferences_find(p,id);if(!visible&&e&&e->kind==RTP_ENTRY_CATEGORY){size_t depth=ui->depth;while(depth>1){const rtp_menu_entry *current=rtp_preferences_find(p,ui->frames[depth-1].category_id);if(current&&current->visible)break;depth--;}ui->depth=depth;}
 return RTP_UI_OK;
}
int rtp_preferences_ui_reset(rtp_preferences_ui *ui,rtp_preferences *p,const char *id){(void)ui;if(!p)return RTP_UI_INVALID_ARGUMENT;return rtp_preferences_reset_entry(p,id)==RTP_PREFS_OK?RTP_UI_OK:RTP_UI_NOT_FOUND;}
int rtp_preferences_ui_apply_profile(rtp_preferences_ui *ui,rtp_preferences *p,rtp_profile_store *profiles,const char *id){
 if(!ui||!p||!profiles)return RTP_UI_INVALID_ARGUMENT;if(rtp_profiles_apply(profiles,id,p)!=RTP_PROFILE_OK)return RTP_UI_NOT_FOUND;rtp_preferences_ui_init(ui);return RTP_UI_OK;
}
