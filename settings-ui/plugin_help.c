#include "plugin_help.h"
#include "plugin_control_centre.h"
#include <string.h>

void rtp_plugin_centre_bind_help(rtp_plugin_centre_snapshot *snapshot,
                                 const rtp_plugin_help_catalogue *help) {
    size_t i;
    if (!snapshot || !help || snapshot->count > RTP_PLUGIN_CENTRE_MAX) return;
    for (i=0;i<snapshot->count;++i) {
        const rtp_plugin_centre_row *row=&snapshot->rows[i];
        snapshot->rows[i].has_help =
            row->plugin_id[0] != '\0' &&
            rtp_plugin_help_find(help,row->plugin_id) != NULL;
    }
}
static int valid_id(const char *s) {
    size_t i;
    if (!s || !s[0]) return 0;
    for (i=0;s[i];++i) {
        char c=s[i];
        if (!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.')) return 0;
    }
    return 1;
}
static int copy_field(char *dst,size_t cap,const char *src,int required) {
    size_t n;
    if (!src) src="";
    n=strlen(src);
    if ((required && !n) || n>=cap) return 0;
    memcpy(dst,src,n+1);
    return 1;
}
void rtp_plugin_help_init(rtp_plugin_help_catalogue *c) {
    if (c) memset(c,0,sizeof(*c));
}
const rtp_plugin_help_entry *rtp_plugin_help_find(
    const rtp_plugin_help_catalogue *c,const char *id) {
    size_t i;
    if (!c || !valid_id(id) || c->count>RTP_HELP_MAX_ENTRIES) return NULL;
    for(i=0;i<c->count;++i) if(strcmp(c->entries[i].plugin_id,id)==0) return &c->entries[i];
    return NULL;
}
rtp_plugin_help_result rtp_plugin_help_register(
    rtp_plugin_help_catalogue *c,const rtp_plugin_help_descriptor *d) {
    rtp_plugin_help_entry e;
    if(!c||!d||!valid_id(d->plugin_id)) return RTP_HELP_INVALID_ARGUMENT;
    if(c->count>RTP_HELP_MAX_ENTRIES) return RTP_HELP_INVALID_ARGUMENT;
    if(c->count==RTP_HELP_MAX_ENTRIES) return RTP_HELP_CAPACITY;
    if(rtp_plugin_help_find(c,d->plugin_id)) return RTP_HELP_DUPLICATE;
    memset(&e,0,sizeof(e));
#define COPY(field,required) if(!copy_field(e.field,sizeof(e.field),d->field,required)) return RTP_HELP_INVALID_TEXT
    COPY(plugin_id,1); COPY(display_name,1); COPY(author,1); COPY(version,0);
    COPY(summary,1); COPY(technical_details,1); COPY(configuration_details,0);
    COPY(dependencies,0); COPY(compatibility,1); COPY(known_conflicts,0);
    COPY(credits,1); COPY(license,0); COPY(source_url,0); COPY(release_url,0);
    COPY(verification_note,1);
#undef COPY
    c->entries[c->count++]=e;
    return RTP_HELP_OK;
}
rtp_plugin_help_result rtp_plugin_help_open(
    const rtp_plugin_help_catalogue *c,const char *id,rtp_plugin_help_panel *p) {
    const rtp_plugin_help_entry *e;
    if(!c||!id||!p) return RTP_HELP_INVALID_ARGUMENT;
    e=rtp_plugin_help_find(c,id);
    if(!e) { memset(p,0,sizeof(*p)); return RTP_HELP_NOT_FOUND; }
    p->entry=e; p->show_help_button=1;
    p->has_source_link=e->source_url[0]!='\0';
    p->has_release_link=e->release_url[0]!='\0';
    return RTP_HELP_OK;
}
rtp_plugin_help_result rtp_plugin_help_register_builtin(rtp_plugin_help_catalogue *c) {
    static const rtp_plugin_help_descriptor custom_warning={
        "custom-warning","Custom Warning (VitaTweaks)","TheOfficialFloW","Not pinned",
        "Replaces the default system warning with a user-defined message or a rotating set of warning presets.",
        "The plugin reads warning text from its taiHEN data locations. The supported format is UTF-16 little-endian with a byte-order mark. The adapter models the plugin's observed precedence: ux0 preset directory, ur0 preset directory, ux0 fixed message, ur0 fixed message, then the system default. Preset rotation is managed by the upstream plugin; this project adapter edits the source text files only.",
        "Fixed-message mode uses custom_warning.txt. Preset-rotation mode uses regular files inside custom_warning/. The adapter checks plugin activation before writes, validates UTF encoding and text bounds, backs up the previous file and verifies the write before accepting it.",
        "VitaTweaks Custom Warning plugin must be installed and enabled in taiHEN. The adapter requires a filesystem backend implementing bounded reads, backups, atomic writes and restore.",
        "Firmware support must be checked against the installed plugin build. The adapter's host tests validate file-format and safety behavior, not live plugin execution on a Vita.",
        "No universal conflict list is established. Multiple override files can affect which warning text wins; the adapter reports the detected source instead of silently editing lower-priority files.",
        "Plugin author: TheOfficialFloW. Adapter implementation and regression tests: Rt:Settings Extended contributors. Respect upstream project attribution.",
        "Upstream license; verify revision before redistribution.",
        "https://github.com/TheOfficialFloW/VitaTweaks",
        "https://github.com/TheOfficialFloW/VitaTweaks",
        "Source project identified; exact installed plugin version and physical-console behavior are not verified by this adapter."
    };
    static const rtp_plugin_help_descriptor wifi_turbo={
        "vita-turbo-wifi","Vita-Turbo-WiFi","Upstream project authors","Not pinned",
        "Provides remote Wi-Fi tuning and status controls for a compatible Vita-Turbo-WiFi plugin instance.",
        "The adapter communicates with the plugin over a bounded request/response transport. It exposes status, keep-Wi-Fi, HUD, auto-reconnect, roaming, performance profiles, HT40, confirmation/revert, roaming and saved-network connection commands. Requests use a configurable host, port and timeout; each command is sent over one transport exchange.",
        "Remote-control operations are gated by an explicit developer-remote-control setting. A transport callback must be supplied by the platform integration. The adapter does not discover devices or open network sockets by itself.",
        "A compatible Vita-Turbo-WiFi plugin endpoint reachable over the configured network, plus a platform transport implementation.",
        "Exact firmware and plugin-version matrix is not pinned in this build. Confirm compatibility against the upstream release installed on the console.",
        "Remote commands may change wireless behavior and connectivity. Network reachability, firewall rules and plugin-side command support can affect results.",
        "Upstream plugin authors retain original credits. Rt:Settings Extended provides the client-side adapter contract and tests.",
        "Upstream license; verify exact source terms.",
        "https://github.com/SilverCrow2323",
        "https://github.com/SilverCrow2323",
        "Adapter API is present; upstream release/version mapping and live Vita network integration remain unverified."
    };
    rtp_plugin_help_result r;
    if(!c) return RTP_HELP_INVALID_ARGUMENT;
    r=rtp_plugin_help_register(c,&custom_warning); if(r!=RTP_HELP_OK) return r;
    return rtp_plugin_help_register(c,&wifi_turbo);
}
