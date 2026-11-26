#include "asset_manifest.h"
#include "sha256.h"
#include <string.h>
#include <stdlib.h>

#define HEADER_SIZE 12U
#define TRAILER_SIZE RTA_MANIFEST_DIGEST_SIZE
static const unsigned char MAGIC[4] = { 'R', 'T', 'A', 'M' };

typedef struct {
    const char *value;
    size_t capacity;
} string_field;

static void put_u16(unsigned char *p, uint16_t v) {
    p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8);
}
static void put_u32(unsigned char *p, uint32_t v) {
    unsigned i; for (i = 0; i < 4; ++i) p[i] = (unsigned char)(v >> (i * 8));
}
static void put_u64(unsigned char *p, uint64_t v) {
    unsigned i; for (i = 0; i < 8; ++i) p[i] = (unsigned char)(v >> (i * 8));
}
static uint16_t get_u16(const unsigned char *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static uint32_t get_u32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint64_t get_u64(const unsigned char *p) {
    uint64_t v = 0; unsigned i;
    for (i = 0; i < 8; ++i) v |= (uint64_t)p[i] << (i * 8);
    return v;
}
static int write_bytes(unsigned char *out, size_t cap, size_t *pos,
                       const void *data, size_t length) {
    if (*pos > cap || length > cap - *pos) return 0;
    if (length) memcpy(out + *pos, data, length);
    *pos += length;
    return 1;
}
static int write_string(unsigned char *out, size_t cap, size_t *pos,
                        const char *value, size_t max_length) {
    size_t n = value ? strlen(value) : 0;
    unsigned char length[2];
    if (n > max_length || n > UINT16_MAX) return 0;
    put_u16(length, (uint16_t)n);
    return write_bytes(out, cap, pos, length, sizeof(length)) &&
           write_bytes(out, cap, pos, value, n);
}
static int read_string(const unsigned char *in, size_t length, size_t *pos,
                       char *out, size_t capacity, int required) {
    uint16_t n;
    size_t i;
    if (*pos > length || length - *pos < 2) return 0;
    n = get_u16(in + *pos); *pos += 2;
    if ((size_t)n >= capacity || *pos > length || (size_t)n > length - *pos ||
        (required && n == 0)) return 0;
    for (i = 0; i < n; ++i) if (in[*pos + i] == 0) return 0;
    if (n) memcpy(out, in + *pos, n);
    out[n] = '\0'; *pos += n;
    return 1;
}
static int read_bytes(const unsigned char *in, size_t length, size_t *pos,
                      void *out, size_t amount) {
    if (*pos > length || amount > length - *pos) return 0;
    if (amount) memcpy(out, in + *pos, amount);
    *pos += amount; return 1;
}
static void registry_to_observations(const rta_asset_registry *r,
                                     rta_asset_observation o[RTA_MAX_ASSETS]) {
    size_t i;
    memset(o, 0, sizeof(*o) * RTA_MAX_ASSETS);
    for (i = 0; i < r->count; ++i) {
        const rta_asset *a = &r->assets[i];
        o[i].id=a->id; o[i].title=a->title; o[i].path=a->path;
        o[i].owner=a->owner; o[i].source=a->source; o[i].format=a->format;
        o[i].sha256=a->sha256; o[i].type=a->type; o[i].state=a->state;
        o[i].size_bytes=a->size_bytes; o[i].width=a->width; o[i].height=a->height;
        o[i].duration_ms=a->duration_ms; o[i].sample_rate_hz=a->sample_rate_hz;
        o[i].channels=a->channels; o[i].managed=a->managed;
        o[i].compatible=a->compatible; o[i].hash_verified=a->hash_verified;
        o[i].protected_asset=a->protected_asset;
    }
}

rta_result rta_asset_manifest_encode(const rta_asset_registry *registry,
                                     unsigned char *buffer, size_t capacity,
                                     size_t *written) {
    rta_asset_observation observations[RTA_MAX_ASSETS];
    rta_asset_registry checked;
    size_t pos = HEADER_SIZE, i;
    unsigned char numeric[31], digest[32];
    ts_sha256_ctx hash;
    if (!registry || !buffer || !written) return RTA_ERR_ARGUMENT;
    *written = 0;
    if (registry->count > RTA_MAX_ASSETS) return RTA_ERR_CAPACITY;
    if (capacity > RTA_MANIFEST_MAX_SIZE) capacity = RTA_MANIFEST_MAX_SIZE;
    registry_to_observations(registry, observations);
    if (rta_asset_registry_build(observations, registry->count, &checked) != RTA_OK)
        return RTA_ERR_INVALID_ASSET;
    if (capacity < HEADER_SIZE + TRAILER_SIZE) return RTA_ERR_CAPACITY;
    memcpy(buffer, MAGIC, sizeof(MAGIC));
    put_u16(buffer + 4, RTA_MANIFEST_VERSION);
    put_u16(buffer + 6, (uint16_t)registry->count);
    put_u32(buffer + 8, 0);
    for (i = 0; i < registry->count; ++i) {
        const rta_asset *a = &checked.assets[i];
        numeric[0]=(unsigned char)a->type; numeric[1]=(unsigned char)a->state;
        numeric[2]=(unsigned char)((a->managed ? 1U : 0U) |
                     (a->compatible ? 2U : 0U) | (a->hash_verified ? 4U : 0U) |
                     (a->protected_asset ? 8U : 0U));
        put_u64(numeric + 3, a->size_bytes);
        put_u32(numeric + 11, a->width); put_u32(numeric + 15, a->height);
        put_u32(numeric + 19, a->duration_ms); put_u32(numeric + 23, a->sample_rate_hz);
        put_u32(numeric + 27, a->channels);
        if (!write_bytes(buffer, capacity - TRAILER_SIZE, &pos, numeric, sizeof(numeric)) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->id, RTA_ID_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->title, RTA_TITLE_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->path, RTA_PATH_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->owner, RTA_OWNER_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->source, RTA_SOURCE_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->format, RTA_FORMAT_MAX) ||
            !write_string(buffer, capacity - TRAILER_SIZE, &pos, a->sha256, RTA_SHA256_HEX))
            return RTA_ERR_CAPACITY;
    }
    put_u32(buffer + 8, (uint32_t)(pos - HEADER_SIZE));
    ts_sha256_init(&hash); ts_sha256_update(&hash, buffer, pos);
    ts_sha256_final(&hash, digest);
    memcpy(buffer + pos, digest, sizeof(digest)); pos += sizeof(digest);
    *written = pos;
    return RTA_OK;
}

rta_result rta_asset_manifest_decode(const unsigned char *buffer, size_t length,
                                     rta_asset_registry *registry) {
    rta_asset_registry *decoded;
    size_t pos = HEADER_SIZE, payload_end, i, j;
    uint16_t count;
    uint32_t payload_length;
    unsigned char digest[32], actual[32], numeric[31];
    ts_sha256_ctx hash;
    if (!buffer || !registry) return RTA_ERR_ARGUMENT;
    if (length < HEADER_SIZE + TRAILER_SIZE || length > RTA_MANIFEST_MAX_SIZE)
        return RTA_ERR_INTEGRITY;
    if (memcmp(buffer, MAGIC, sizeof(MAGIC)) != 0 ||
        get_u16(buffer + 4) != RTA_MANIFEST_VERSION) return RTA_ERR_INTEGRITY;
    count = get_u16(buffer + 6); payload_length = get_u32(buffer + 8);
    if (count > RTA_MAX_ASSETS || payload_length != length - HEADER_SIZE - TRAILER_SIZE)
        return RTA_ERR_INTEGRITY;
    payload_end = HEADER_SIZE + payload_length;
    ts_sha256_init(&hash); ts_sha256_update(&hash, buffer, payload_end);
    ts_sha256_final(&hash, actual);
    memcpy(digest, buffer + payload_end, sizeof(digest));
    if (memcmp(digest, actual, sizeof(digest)) != 0) return RTA_ERR_INTEGRITY;

    decoded = (rta_asset_registry *)calloc(1, sizeof(*decoded));
    if (!decoded) return RTA_ERR_CAPACITY;
    for (i = 0; i < count; ++i) {
        rta_asset *a = &decoded->assets[i];
        rta_asset_observation o;
        if (!read_bytes(buffer, payload_end, &pos, numeric, sizeof(numeric))) {
            free(decoded); return RTA_ERR_INTEGRITY;
        }
        a->type=(rta_asset_type)numeric[0]; a->state=(rta_asset_state)numeric[1];
        if (numeric[2] & 0xF0U) { free(decoded); return RTA_ERR_INTEGRITY; }
        a->managed=(numeric[2] & 1U)!=0; a->compatible=(numeric[2] & 2U)!=0;
        a->hash_verified=(numeric[2] & 4U)!=0; a->protected_asset=(numeric[2] & 8U)!=0;
        a->size_bytes=get_u64(numeric+3); a->width=get_u32(numeric+11);
        a->height=get_u32(numeric+15); a->duration_ms=get_u32(numeric+19);
        a->sample_rate_hz=get_u32(numeric+23); a->channels=get_u32(numeric+27);
        if (!read_string(buffer,payload_end,&pos,a->id,sizeof(a->id),1) ||
            !read_string(buffer,payload_end,&pos,a->title,sizeof(a->title),1) ||
            !read_string(buffer,payload_end,&pos,a->path,sizeof(a->path),1) ||
            !read_string(buffer,payload_end,&pos,a->owner,sizeof(a->owner),1) ||
            !read_string(buffer,payload_end,&pos,a->source,sizeof(a->source),0) ||
            !read_string(buffer,payload_end,&pos,a->format,sizeof(a->format),1) ||
            !read_string(buffer,payload_end,&pos,a->sha256,sizeof(a->sha256),0)) {
            free(decoded); return RTA_ERR_INTEGRITY;
        }
        memset(&o, 0, sizeof(o));
        o.id=a->id; o.title=a->title; o.path=a->path; o.owner=a->owner;
        o.source=a->source; o.format=a->format; o.sha256=a->sha256;
        o.type=a->type; o.state=a->state; o.size_bytes=a->size_bytes;
        o.width=a->width; o.height=a->height; o.duration_ms=a->duration_ms;
        o.sample_rate_hz=a->sample_rate_hz; o.channels=a->channels;
        o.managed=a->managed; o.compatible=a->compatible;
        o.hash_verified=a->hash_verified; o.protected_asset=a->protected_asset;
        if (rta_asset_validate(&o) != RTA_OK) { free(decoded); return RTA_ERR_INTEGRITY; }
        for (j = 0; j < i; ++j) {
            if (strcmp(decoded->assets[j].id, a->id) == 0) {
                free(decoded); return RTA_ERR_INTEGRITY;
            }
            if (strcmp(decoded->assets[j].path, a->path) == 0) {
                free(decoded); return RTA_ERR_INTEGRITY;
            }
        }
        if (a->state == RTA_STATE_ACTIVE) decoded->active_count++;
        if (a->state == RTA_STATE_BACKUP) decoded->backup_count++;
        if (a->state == RTA_STATE_ORPHANED) decoded->orphaned_count++;
        if (a->state == RTA_STATE_INCOMPATIBLE) decoded->incompatible_count++;
        if (!a->hash_verified) decoded->unverified_count++;
        decoded->count++;
    }
    if (pos != payload_end) { free(decoded); return RTA_ERR_INTEGRITY; }
    for (i = 0; i < decoded->count; ++i) {
        if (decoded->assets[i].state != RTA_STATE_ACTIVE) continue;
        for (j = i + 1; j < decoded->count; ++j)
            if (decoded->assets[j].state == RTA_STATE_ACTIVE &&
                decoded->assets[i].type == decoded->assets[j].type)
                decoded->active_conflict_count++;
    }
    *registry = *decoded;
    free(decoded);
    return RTA_OK;
}

rta_result rta_asset_manifest_save(const rta_asset_registry *registry,
                                   const rta_asset_manifest_io *io,
                                   const char *path,
                                   unsigned char *scratch, size_t capacity) {
    size_t written = 0;
    rta_result result;
    if (!io || !io->replace_atomic || !path || !scratch) return RTA_ERR_ARGUMENT;
    result = rta_asset_manifest_encode(registry, scratch, capacity, &written);
    if (result != RTA_OK) return result;
    return io->replace_atomic(io->context, path, scratch, written) == 0 ? RTA_OK : RTA_ERR_BACKEND;
}

rta_result rta_asset_manifest_load(const rta_asset_manifest_io *io,
                                   const char *path,
                                   unsigned char *scratch, size_t capacity,
                                   rta_asset_registry *registry) {
    size_t length = 0;
    if (!io || !io->read || !path || !scratch || !registry) return RTA_ERR_ARGUMENT;
    if (io->read(io->context, path, scratch, capacity, &length) != 0)
        return RTA_ERR_BACKEND;
    if (length > capacity) return RTA_ERR_CAPACITY;
    return rta_asset_manifest_decode(scratch, length, registry);
}
