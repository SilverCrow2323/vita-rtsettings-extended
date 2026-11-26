#include "custom_assets.h"
#include <ctype.h>
#include <string.h>

#define RTA_BOOT_SOUND_MAX (1024ULL * 1024ULL)
#define RTA_BOOT_ANIMATION_MAX (108ULL * 1024ULL * 1024ULL)
#define RTA_BOOT_SPLASH_MAX (32ULL * 1024ULL * 1024ULL)
#define RTA_WARNING_MAX (16ULL * 1024ULL)
#define RTA_THEME_MAX (32ULL * 1024ULL * 1024ULL)
#define RTA_OTHER_MAX (128ULL * 1024ULL * 1024ULL)

static int copy_text(char *dst, size_t capacity, const char *src, int required) {
    size_t length;
    if (!dst || !capacity) return 0;
    if (!src) { dst[0] = '\0'; return !required; }
    length = strlen(src);
    if ((required && !length) || length >= capacity) return 0;
    memcpy(dst, src, length + 1);
    return 1;
}

static int valid_id(const char *id) {
    size_t i, n;
    if (!id || !(n = strlen(id)) || n > RTA_ID_MAX) return 0;
    if (!isalnum((unsigned char)id[0])) return 0;
    for (i = 0; i < n; ++i)
        if (!isalnum((unsigned char)id[i]) && id[i] != '-' && id[i] != '_') return 0;
    return 1;
}

static int valid_path(const char *path) {
    size_t i, n;
    int root_ok = 0;
    if (!path || !(n = strlen(path)) || n > RTA_PATH_MAX) return 0;
    if (strncmp(path, "ur0:", 4) == 0 || strncmp(path, "ux0:", 4) == 0 ||
        strncmp(path, "uma0:", 5) == 0 || strncmp(path, "imc0:", 5) == 0 ||
        strncmp(path, "app0:", 5) == 0) root_ok = 1;
    if (!root_ok) return 0;
    for (i = 0; i < n; ++i) {
        unsigned char ch = (unsigned char)path[i];
        if (ch < 32 || ch == 127) return 0;
        if (path[i] == '.' && i + 1 < n && path[i + 1] == '.') return 0;
    }
    return 1;
}

static int valid_hash(const char *hash) {
    size_t i;
    if (!hash || strlen(hash) != RTA_SHA256_HEX) return 0;
    for (i = 0; i < RTA_SHA256_HEX; ++i)
        if (!((hash[i] >= '0' && hash[i] <= '9') || (hash[i] >= 'a' && hash[i] <= 'f'))) return 0;
    return 1;
}

rta_result rta_asset_validate(const rta_asset_observation *o) {
    uint64_t max_size;
    if (!o || !valid_id(o->id) || !o->title || !o->title[0] ||
        strlen(o->title) > RTA_TITLE_MAX || !valid_path(o->path) ||
        !o->owner || !o->owner[0] || strlen(o->owner) > RTA_OWNER_MAX ||
        !o->format || !o->format[0] || strlen(o->format) > RTA_FORMAT_MAX ||
        o->type < RTA_TYPE_BOOT_SPLASH || o->type > RTA_TYPE_OTHER ||
        o->state < RTA_STATE_AVAILABLE || o->state > RTA_STATE_INVALID ||
        !o->size_bytes) return RTA_ERR_INVALID_ASSET;

    switch (o->type) {
        case RTA_TYPE_BOOT_SPLASH: max_size = RTA_BOOT_SPLASH_MAX; break;
        case RTA_TYPE_BOOT_ANIMATION: max_size = RTA_BOOT_ANIMATION_MAX; break;
        case RTA_TYPE_BOOT_SOUND: max_size = RTA_BOOT_SOUND_MAX; break;
        case RTA_TYPE_STARTUP_WARNING: max_size = RTA_WARNING_MAX; break;
        case RTA_TYPE_INTERFACE_THEME: max_size = RTA_THEME_MAX; break;
        default: max_size = RTA_OTHER_MAX; break;
    }
    if (o->size_bytes > max_size) return RTA_ERR_INVALID_ASSET;
    if ((o->type == RTA_TYPE_BOOT_SPLASH || o->type == RTA_TYPE_BOOT_ANIMATION) &&
        (!o->width || !o->height)) return RTA_ERR_INVALID_ASSET;
    if (o->type == RTA_TYPE_BOOT_SOUND && !o->duration_ms) return RTA_ERR_INVALID_ASSET;
    if (o->hash_verified && !valid_hash(o->sha256)) return RTA_ERR_INVALID_ASSET;
    if (o->sha256 && o->sha256[0] && !valid_hash(o->sha256)) return RTA_ERR_INVALID_ASSET;
    if (o->managed && !o->owner[0]) return RTA_ERR_INVALID_ASSET;
    return RTA_OK;
}

static void copy_observation(const rta_asset_observation *o, rta_asset *a) {
    (void)copy_text(a->id, sizeof(a->id), o->id, 1);
    (void)copy_text(a->title, sizeof(a->title), o->title, 1);
    (void)copy_text(a->path, sizeof(a->path), o->path, 1);
    (void)copy_text(a->owner, sizeof(a->owner), o->owner, 1);
    (void)copy_text(a->format, sizeof(a->format), o->format, 1);
    (void)copy_text(a->sha256, sizeof(a->sha256), o->sha256, 0);
    a->type = o->type; a->state = o->state; a->size_bytes = o->size_bytes;
    a->width = o->width; a->height = o->height; a->duration_ms = o->duration_ms;
    a->managed = o->managed ? 1 : 0; a->compatible = o->compatible ? 1 : 0;
    a->hash_verified = o->hash_verified ? 1 : 0;
    a->protected_asset = o->protected_asset ? 1 : 0;
}

rta_result rta_asset_registry_build(const rta_asset_observation *observations,
                                    size_t count, rta_asset_registry *out) {
    size_t i, j;
    if (!out || (!observations && count)) return RTA_ERR_ARGUMENT;
    if (count > RTA_MAX_ASSETS) return RTA_ERR_CAPACITY;
    memset(out, 0, sizeof(*out));
    for (i = 0; i < count; ++i) {
        rta_asset *asset;
        rta_result valid = rta_asset_validate(&observations[i]);
        if (valid != RTA_OK) return valid;
        asset = &out->assets[i];
        for (j = 0; j < i; ++j) {
            if (strcmp(out->assets[j].id, observations[i].id) == 0) return RTA_ERR_DUPLICATE_ID;
            if (strcmp(out->assets[j].path, observations[i].path) == 0) return RTA_ERR_DUPLICATE_PATH;
        }
        copy_observation(&observations[i], asset);
        if (asset->state == RTA_STATE_ACTIVE) out->active_count++;
        if (asset->state == RTA_STATE_BACKUP) out->backup_count++;
        if (asset->state == RTA_STATE_ORPHANED) out->orphaned_count++;
        if (asset->state == RTA_STATE_INCOMPATIBLE) out->incompatible_count++;
        if (!asset->hash_verified) out->unverified_count++;
        out->count++;
    }
    for (i = 0; i < out->count; ++i) {
        if (out->assets[i].state != RTA_STATE_ACTIVE) continue;
        for (j = i + 1; j < out->count; ++j)
            if (out->assets[j].state == RTA_STATE_ACTIVE &&
                out->assets[i].type == out->assets[j].type) out->active_conflict_count++;
    }
    return RTA_OK;
}

static int find_asset(const rta_asset_registry *registry, const char *id) {
    size_t i;
    if (!registry || !id) return -1;
    for (i = 0; i < registry->count; ++i)
        if (strcmp(registry->assets[i].id, id) == 0) return (int)i;
    return -1;
}

rta_result rta_asset_activate(rta_asset_registry *registry, const char *asset_id,
                              const rta_asset_backend *backend) {
    int target_index, previous_index = -1;
    size_t i;
    rta_asset *target;
    const char *previous_path = NULL;
    if (!registry || !asset_id || !backend || !backend->activate ||
        !backend->verify_active || !backend->restore) return RTA_ERR_ARGUMENT;
    target_index = find_asset(registry, asset_id);
    if (target_index < 0) return RTA_ERR_NOT_FOUND;
    target = &registry->assets[target_index];
    if (target->state == RTA_STATE_ACTIVE) return RTA_OK;
    if (!target->managed) return RTA_ERR_UNSAFE;
    if (target->state == RTA_STATE_ORPHANED || target->state == RTA_STATE_INVALID ||
        target->state == RTA_STATE_INCOMPATIBLE || !target->compatible) return RTA_ERR_INCOMPATIBLE;
    if (!target->hash_verified || !valid_hash(target->sha256)) return RTA_ERR_UNVERIFIED;
    for (i = 0; i < registry->count; ++i) {
        if (i != (size_t)target_index && registry->assets[i].type == target->type &&
            registry->assets[i].state == RTA_STATE_ACTIVE) {
            if (previous_index >= 0) return RTA_ERR_UNSAFE;
            previous_index = (int)i;
            previous_path = registry->assets[i].path;
        }
    }
    if (backend->activate(backend->context, target->type, target->path) != 0 ||
        backend->verify_active(backend->context, target->type, target->path) != 0) {
        if (backend->restore(backend->context, target->type, previous_path) != 0) return RTA_ERR_ROLLBACK;
        return RTA_ERR_BACKEND;
    }
    if (previous_index >= 0) registry->assets[previous_index].state = RTA_STATE_AVAILABLE;
    target->state = RTA_STATE_ACTIVE;
    registry->active_count = 0;
    for (i = 0; i < registry->count; ++i)
        if (registry->assets[i].state == RTA_STATE_ACTIVE) registry->active_count++;
    registry->active_conflict_count = 0;
    for (i = 0; i < registry->count; ++i) {
        size_t j;
        if (registry->assets[i].state != RTA_STATE_ACTIVE) continue;
        for (j = i + 1; j < registry->count; ++j)
            if (registry->assets[j].state == RTA_STATE_ACTIVE &&
                registry->assets[i].type == registry->assets[j].type) registry->active_conflict_count++;
    }
    return RTA_OK;
}

rta_result rta_asset_remove(rta_asset_registry *registry, const char *asset_id,
                            const rta_asset_backend *backend) {
    int index;
    rta_asset *asset;
    if (!registry || !asset_id || !backend || !backend->remove) return RTA_ERR_ARGUMENT;
    index = find_asset(registry, asset_id);
    if (index < 0) return RTA_ERR_NOT_FOUND;
    asset = &registry->assets[index];
    if (!asset->managed || asset->state == RTA_STATE_ORPHANED ||
        asset->state == RTA_STATE_ACTIVE) return RTA_ERR_UNSAFE;
    if (asset->protected_asset || asset->state == RTA_STATE_BACKUP) return RTA_ERR_PROTECTED;
    {
        int was_unverified = !asset->hash_verified;
        char path[RTA_PATH_MAX + 1];
        memcpy(path, asset->path, sizeof(path));
        path[sizeof(path) - 1] = '\\0';
        if (backend->remove(backend->context, path) != 0) return RTA_ERR_BACKEND;
        if ((size_t)index + 1 < registry->count)
            memmove(&registry->assets[index], &registry->assets[index + 1],
                    (registry->count - (size_t)index - 1) * sizeof(registry->assets[0]));
        memset(&registry->assets[registry->count - 1], 0, sizeof(registry->assets[0]));
        registry->count--;
        if (was_unverified && registry->unverified_count) registry->unverified_count--;
    }
    return RTA_OK;
}

const char *rta_asset_type_name(rta_asset_type t) {
    switch (t) {
        case RTA_TYPE_BOOT_SPLASH: return "boot-splash";
        case RTA_TYPE_BOOT_ANIMATION: return "boot-animation";
        case RTA_TYPE_BOOT_SOUND: return "boot-sound";
        case RTA_TYPE_STARTUP_WARNING: return "startup-warning";
        case RTA_TYPE_INTERFACE_THEME: return "interface-theme";
        case RTA_TYPE_OTHER: return "other";
        default: return "invalid";
    }
}
const char *rta_asset_state_name(rta_asset_state s) {
    switch (s) {
        case RTA_STATE_AVAILABLE: return "available";
        case RTA_STATE_ACTIVE: return "active";
        case RTA_STATE_BACKUP: return "backup";
        case RTA_STATE_ORPHANED: return "orphaned";
        case RTA_STATE_INCOMPATIBLE: return "incompatible";
        case RTA_STATE_INVALID: return "invalid";
        default: return "invalid";
    }
}
const char *rta_result_name(rta_result r) {
    switch (r) {
        case RTA_OK: return "ok";
        case RTA_ERR_ARGUMENT: return "invalid-argument";
        case RTA_ERR_CAPACITY: return "capacity";
        case RTA_ERR_INVALID_ASSET: return "invalid-asset";
        case RTA_ERR_DUPLICATE_ID: return "duplicate-id";
        case RTA_ERR_DUPLICATE_PATH: return "duplicate-path";
        case RTA_ERR_NOT_FOUND: return "not-found";
        case RTA_ERR_UNSAFE: return "unsafe-operation";
        case RTA_ERR_INCOMPATIBLE: return "incompatible";
        case RTA_ERR_UNVERIFIED: return "unverified";
        case RTA_ERR_BACKEND: return "backend-error";
        case RTA_ERR_ROLLBACK: return "rollback-failed";
        case RTA_ERR_PROTECTED: return "protected";
        default: return "unknown";
    }
}
