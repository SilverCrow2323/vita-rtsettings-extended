#include "custom_warning_adapter.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int safe_name(const char *name) {
    size_t i, n;
    if (!name || !(n = strlen(name)) || n > RTCW_PRESET_NAME_MAX ||
        name[0] == '.' || strstr(name, "..")) return 0;
    for (i = 0; i < n; ++i) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

static int safe_mount(const char *mount) {
    return mount && (strcmp(mount, "ux0") == 0 || strcmp(mount, "ur0") == 0);
}

static rtcw_result utf8_next(const unsigned char *s, size_t n, size_t *used, uint32_t *cp) {
    unsigned char a;
    if (!s || !n || !used || !cp) return RTCW_ERR_INVALID_TEXT;
    a = s[0];
    if (a < 0x80) { *cp = a; *used = 1; }
    else if (a >= 0xC2 && a <= 0xDF && n >= 2 && (s[1] & 0xC0) == 0x80) {
        *cp = ((uint32_t)(a & 0x1F) << 6) | (s[1] & 0x3F); *used = 2;
    } else if (a >= 0xE0 && a <= 0xEF && n >= 3 &&
               (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 &&
               !(a == 0xE0 && s[1] < 0xA0) && !(a == 0xED && s[1] >= 0xA0)) {
        *cp = ((uint32_t)(a & 0x0F) << 12) | ((uint32_t)(s[1] & 0x3F) << 6) | (s[2] & 0x3F); *used = 3;
    } else if (a >= 0xF0 && a <= 0xF4 && n >= 4 &&
               (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80 &&
               !(a == 0xF0 && s[1] < 0x90) && !(a == 0xF4 && s[1] >= 0x90)) {
        *cp = ((uint32_t)(a & 0x07) << 18) | ((uint32_t)(s[1] & 0x3F) << 12) |
              ((uint32_t)(s[2] & 0x3F) << 6) | (s[3] & 0x3F); *used = 4;
    } else return RTCW_ERR_INVALID_TEXT;
    if (*cp == 0 || *cp > 0x10FFFF || (*cp >= 0xD800 && *cp <= 0xDFFF) ||
        (*cp < 0x20 && *cp != 0x09 && *cp != 0x0A && *cp != 0x0D) ||
        (*cp >= 0x7F && *cp <= 0x9F)) return RTCW_ERR_INVALID_TEXT;
    return RTCW_OK;
}

static size_t utf8_put(uint32_t cp, char *out) {
    if (cp < 0x80) { out[0] = (char)cp; return 1; }
    if (cp < 0x800) { out[0]=(char)(0xC0|(cp>>6)); out[1]=(char)(0x80|(cp&0x3F)); return 2; }
    if (cp < 0x10000) { out[0]=(char)(0xE0|(cp>>12)); out[1]=(char)(0x80|((cp>>6)&0x3F)); out[2]=(char)(0x80|(cp&0x3F)); return 3; }
    out[0]=(char)(0xF0|(cp>>18)); out[1]=(char)(0x80|((cp>>12)&0x3F));
    out[2]=(char)(0x80|((cp>>6)&0x3F)); out[3]=(char)(0x80|(cp&0x3F)); return 4;
}

rtcw_result rtcw_encode_utf8(const char *utf8, unsigned char *out, size_t capacity, size_t *out_size) {
    size_t i = 0, pos = 2, length;
    if (!utf8 || !out || !out_size) return RTCW_ERR_ARGUMENT;
    length = strlen(utf8);
    if (length > RTCW_MAX_TEXT_BYTES) return RTCW_ERR_TEXT_TOO_LONG;
    if (capacity < 2) return RTCW_ERR_CAPACITY;
    out[0] = 0xFF; out[1] = 0xFE;
    while (i < length) {
        size_t used = 0;
        uint32_t cp = 0;
        rtcw_result r = utf8_next((const unsigned char *)utf8 + i, length - i, &used, &cp);
        if (r != RTCW_OK) return r;
        if (cp <= 0xFFFF) {
            if (pos + 2 > capacity || pos + 2 > RTCW_MAX_FILE_BYTES) return RTCW_ERR_TEXT_TOO_LONG;
            out[pos++] = (unsigned char)(cp & 0xFF); out[pos++] = (unsigned char)(cp >> 8);
        } else {
            uint32_t value = cp - 0x10000;
            uint16_t high = (uint16_t)(0xD800 | (value >> 10));
            uint16_t low = (uint16_t)(0xDC00 | (value & 0x3FF));
            if (pos + 4 > capacity || pos + 4 > RTCW_MAX_FILE_BYTES) return RTCW_ERR_TEXT_TOO_LONG;
            out[pos++] = (unsigned char)(high & 0xFF); out[pos++] = (unsigned char)(high >> 8);
            out[pos++] = (unsigned char)(low & 0xFF); out[pos++] = (unsigned char)(low >> 8);
        }
        i += used;
    }
    *out_size = pos;
    return RTCW_OK;
}

rtcw_result rtcw_validate_utf16le(const unsigned char *bytes, size_t size) {
    size_t i;
    if (!bytes || size < 2 || size > RTCW_MAX_FILE_BYTES || (size & 1U) || bytes[0] != 0xFF || bytes[1] != 0xFE)
        return RTCW_ERR_INVALID_ENCODING;
    for (i = 2; i < size; i += 2) {
        uint16_t u = (uint16_t)(bytes[i] | ((uint16_t)bytes[i + 1] << 8));
        if (u == 0 || (u < 0x20 && u != 0x09 && u != 0x0A && u != 0x0D) ||
            (u >= 0x7F && u <= 0x9F)) return RTCW_ERR_INVALID_ENCODING;
        if (u >= 0xD800 && u <= 0xDBFF) {
            uint16_t low;
            if (i + 3 >= size) return RTCW_ERR_INVALID_ENCODING;
            low = (uint16_t)(bytes[i + 2] | ((uint16_t)bytes[i + 3] << 8));
            if (low < 0xDC00 || low > 0xDFFF) return RTCW_ERR_INVALID_ENCODING;
            i += 2;
        } else if (u >= 0xDC00 && u <= 0xDFFF) return RTCW_ERR_INVALID_ENCODING;
    }
    return RTCW_OK;
}

rtcw_result rtcw_decode_utf16le(const unsigned char *bytes, size_t size,
                                char *utf8, size_t capacity, size_t *out_size) {
    size_t i, pos = 0;
    if (!utf8 || !capacity || !out_size) return RTCW_ERR_ARGUMENT;
    if (rtcw_validate_utf16le(bytes, size) != RTCW_OK) return RTCW_ERR_INVALID_ENCODING;
    for (i = 2; i < size; i += 2) {
        uint32_t cp = (uint32_t)bytes[i] | ((uint32_t)bytes[i + 1] << 8);
        if (cp >= 0xD800 && cp <= 0xDBFF) {
            uint32_t low = (uint32_t)bytes[i + 2] | ((uint32_t)bytes[i + 3] << 8);
            cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
            i += 2;
        }
        {
            char encoded[4];
            size_t n = utf8_put(cp, encoded);
            if (pos + n >= capacity || pos + n > RTCW_MAX_TEXT_BYTES) return RTCW_ERR_TEXT_TOO_LONG;
            memcpy(utf8 + pos, encoded, n); pos += n;
        }
    }
    utf8[pos] = '\0'; *out_size = pos;
    return RTCW_OK;
}

static rtcw_result read_file(const rtcw_backend *b, const char *path, unsigned char *buf, size_t *size) {
    int r;
    if (!b->read_file) return RTCW_ERR_ARGUMENT;
    *size = 0;
    r = b->read_file(b->context, path, buf, RTCW_MAX_FILE_BYTES, size);
    if (r < 0) return RTCW_ERR_BACKEND;
    if (r == 0) return RTCW_ERR_NOT_FOUND;
    if (r != 1 || *size > RTCW_MAX_FILE_BYTES) return RTCW_ERR_BACKEND;
    return RTCW_OK;
}

static rtcw_result build_path(char *out, size_t cap, const char *mount, const char *suffix) {
    int n;
    if (!safe_mount(mount) || !suffix || !out) return RTCW_ERR_ARGUMENT;
    n = snprintf(out, cap, "%s:tai/%s", mount, suffix);
    return n < 0 || (size_t)n >= cap ? RTCW_ERR_CAPACITY : RTCW_OK;
}

static rtcw_result transactional_write(const rtcw_backend *b, const char *path,
                                       const unsigned char *bytes, size_t size) {
    unsigned char verify[RTCW_MAX_FILE_BYTES];
    size_t actual_size = 0;
    rtcw_result read_result;
    if (!b || !b->backup_file || !b->write_atomic || !b->restore_backup || !b->read_file)
        return RTCW_ERR_ARGUMENT;
    if (b->backup_file(b->context, path) != 0) return RTCW_ERR_BACKEND;
    if (b->write_atomic(b->context, path, bytes, size) != 0) {
        if (b->restore_backup(b->context, path) != 0) return RTCW_ERR_ROLLBACK;
        return RTCW_ERR_BACKEND;
    }
    read_result = read_file(b, path, verify, &actual_size);
    if (read_result != RTCW_OK || actual_size != size || memcmp(verify, bytes, size) != 0) {
        if (b->restore_backup(b->context, path) != 0) return RTCW_ERR_ROLLBACK;
        return read_result == RTCW_OK ? RTCW_ERR_INTEGRITY : RTCW_ERR_BACKEND;
    }
    return RTCW_OK;
}

rtcw_result rtcw_read_state(const rtcw_backend *b, rtcw_state *out) {
    char dir[RTCW_PATH_MAX + 1], file[RTCW_PATH_MAX + 1];
    char names[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX + 1];
    unsigned char raw[RTCW_MAX_FILE_BYTES];
    size_t count = 0, size = 0, i;
    int active, listed;
    const char *mounts[] = {"ux0", "ur0"};
    if (!b || !out || !b->plugin_active || !b->list_directory || !b->read_file) return RTCW_ERR_ARGUMENT;
    memset(out, 0, sizeof(*out));
    active = b->plugin_active(b->context);
    if (active == 0) return RTCW_ERR_PLUGIN_DISABLED;
    if (active != 1) return RTCW_ERR_BACKEND;
    for (i = 0; i < 2; ++i) {
        if (build_path(dir, sizeof(dir), mounts[i], "custom_warning") != RTCW_OK) return RTCW_ERR_CAPACITY;
        count = 0; listed = b->list_directory(b->context, dir, names, RTCW_MAX_PRESETS, &count);
        if (listed < 0 || count > RTCW_MAX_PRESETS) return RTCW_ERR_BACKEND;
        if (listed == 1 && count > 0) {
            size_t j;
            out->mode = RTCW_MODE_PRESET_ROTATION; strcpy(out->mount, mounts[i]);
            strcpy(out->path, dir); out->preset_count = count;
            for (j = 0; j < count; ++j) {
                if (!safe_name(names[j])) return RTCW_ERR_BACKEND;
                strcpy(out->preset_names[j], names[j]);
            }
            return RTCW_OK;
        }
    }
    for (i = 0; i < 2; ++i) {
        if (build_path(file, sizeof(file), mounts[i], "custom_warning.txt") != RTCW_OK) return RTCW_ERR_CAPACITY;
        size = 0;
        {
            int r = b->read_file(b->context, file, raw, sizeof(raw), &size);
            if (r < 0) return RTCW_ERR_BACKEND;
            if (r == 0) continue;
            if (r != 1 || size > sizeof(raw)) return RTCW_ERR_BACKEND;
        }
        out->mode = RTCW_MODE_FIXED_FILE; strcpy(out->mount, mounts[i]); strcpy(out->path, file);
        return rtcw_decode_utf16le(raw, size, out->current_text, sizeof(out->current_text), &out->current_text_bytes);
    }
    out->mode = RTCW_MODE_DEFAULT; strcpy(out->mount, "ux0");
    return build_path(out->path, sizeof(out->path), "ux0", "custom_warning.txt");
}

rtcw_result rtcw_read_preset(const rtcw_backend *b, const char *mount, const char *name,
                             char *utf8, size_t capacity, size_t *out_size) {
    char suffix[RTCW_PRESET_NAME_MAX + 32], path[RTCW_PATH_MAX + 1];
    unsigned char raw[RTCW_MAX_FILE_BYTES]; size_t size = 0; int n;
    if (!b || !b->read_file || !safe_mount(mount) || !safe_name(name) || !utf8) return RTCW_ERR_ARGUMENT;
    n = snprintf(suffix, sizeof(suffix), "custom_warning/%s", name);
    if (n < 0 || (size_t)n >= sizeof(suffix)) return RTCW_ERR_CAPACITY;
    if (build_path(path, sizeof(path), mount, suffix) != RTCW_OK) return RTCW_ERR_CAPACITY;
    {
        rtcw_result r = read_file(b, path, raw, &size);
        if (r != RTCW_OK) return r;
    }
    return rtcw_decode_utf16le(raw, size, utf8, capacity, out_size);
}

rtcw_result rtcw_write_preset(const rtcw_backend *b, const char *mount,
                              const char *name, const char *utf8, int add_if_missing) {
    char suffix[RTCW_PRESET_NAME_MAX + 32], path[RTCW_PATH_MAX + 1];
    unsigned char encoded[RTCW_MAX_FILE_BYTES]; size_t size = 0; int n;
    if (!b || !b->read_file || !safe_mount(mount) || !safe_name(name) || !utf8 ||
        (add_if_missing != 0 && add_if_missing != 1)) return RTCW_ERR_ARGUMENT;
    n = snprintf(suffix, sizeof(suffix), "custom_warning/%s", name);
    if (n < 0 || (size_t)n >= sizeof(suffix)) return RTCW_ERR_CAPACITY;
    if (build_path(path, sizeof(path), mount, suffix) != RTCW_OK) return RTCW_ERR_CAPACITY;
    {
        unsigned char existing[RTCW_MAX_FILE_BYTES]; size_t existing_size = 0;
        int exists = b->read_file(b->context, path, existing, sizeof(existing), &existing_size);
        if (exists < 0) return RTCW_ERR_BACKEND;
        if (exists == 1 && add_if_missing) return RTCW_ERR_MODE;
        if (exists == 0 && !add_if_missing) return RTCW_ERR_NOT_FOUND;
    }
    {
        rtcw_result r = rtcw_encode_utf8(utf8, encoded, sizeof(encoded), &size);
        if (r != RTCW_OK) return r;
    }
    return transactional_write(b, path, encoded, size);
}

rtcw_result rtcw_write_message(const rtcw_backend *b, const char *utf8) {
    rtcw_state state; unsigned char encoded[RTCW_MAX_FILE_BYTES]; size_t size = 0;
    rtcw_result r;
    if (!b || !utf8) return RTCW_ERR_ARGUMENT;
    r = rtcw_read_state(b, &state);
    if (r != RTCW_OK) return r;
    if (state.mode == RTCW_MODE_PRESET_ROTATION) return RTCW_ERR_MODE;
    r = rtcw_encode_utf8(utf8, encoded, sizeof(encoded), &size);
    if (r != RTCW_OK) return r;
    return transactional_write(b, state.path, encoded, size);
}

rtcw_result rtcw_restore_original(const rtcw_backend *b, const char *path) {
    if (!b || !b->restore_backup || !path ||
        (strncmp(path, "ux0:tai/custom_warning", 22) != 0 &&
         strncmp(path, "ur0:tai/custom_warning", 22) != 0)) return RTCW_ERR_ARGUMENT;
    return b->restore_backup(b->context, path) == 0 ? RTCW_OK : RTCW_ERR_BACKEND;
}

const char *rtcw_mode_name(rtcw_mode m) {
    switch (m) { case RTCW_MODE_DEFAULT: return "default"; case RTCW_MODE_FIXED_FILE: return "fixed-file"; case RTCW_MODE_PRESET_ROTATION: return "preset-rotation"; default: return "invalid"; }
}
const char *rtcw_result_name(rtcw_result r) {
    switch (r) {
        case RTCW_OK: return "ok"; case RTCW_ERR_ARGUMENT: return "invalid-argument";
        case RTCW_ERR_PLUGIN_DISABLED: return "plugin-disabled"; case RTCW_ERR_NOT_FOUND: return "not-found";
        case RTCW_ERR_BACKEND: return "backend-error"; case RTCW_ERR_INVALID_ENCODING: return "invalid-encoding";
        case RTCW_ERR_TEXT_TOO_LONG: return "text-too-long"; case RTCW_ERR_INVALID_TEXT: return "invalid-text";
        case RTCW_ERR_MODE: return "mode-conflict"; case RTCW_ERR_ROLLBACK: return "rollback-failed";
        case RTCW_ERR_INTEGRITY: return "integrity-mismatch"; case RTCW_ERR_CAPACITY: return "capacity";
        default: return "unknown";
    }
}
