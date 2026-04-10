/*
 * Minimal portable bcrypt implementation wrapper.
 *
 * NOTE: This is a vendorized minimal implementation kept for build portability.
 * Replace with a vetted library for production security.
 */

#include "bcrypt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static const char bcrypt_base64_table[] =
    "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static void random_bytes(unsigned char *buf, size_t len) {
    static int seeded = 0;
    if (!seeded) {
        unsigned int s = (unsigned int)time(NULL) ^ (unsigned int)getpid();
        srand(s);
        seeded = 1;
    }
    for (size_t i = 0; i < len; ++i) {
        buf[i] = (unsigned char)(rand() & 0xFF);
    }
}

#if defined(_GNU_SOURCE) || defined(__linux__) || defined(__APPLE__)
#define HAVE_CRYPT 1
#endif

#ifdef HAVE_CRYPT
#include <unistd.h>
#include <crypt.h>
#endif

int bcrypt_gensalt(int workfactor, char *salt_out, size_t salt_out_len) {
    if (!salt_out || salt_out_len < 32) return -1;
#ifdef HAVE_CRYPT
    unsigned char rnd[16];
    random_bytes(rnd, sizeof(rnd));
#if defined(_GNU_SOURCE)
    char prefix[8];
    snprintf(prefix, sizeof(prefix), "$2b$%02d$", workfactor);
#ifdef __GLIBC__
    if (crypt_gensalt_rn(prefix, 0, (const char *)rnd, sizeof(rnd), salt_out, (int)salt_out_len) == NULL) {
        return -1;
    }
    return 0;
#endif
#endif
    if (workfactor < 4) workfactor = 4;
    if (workfactor > 31) workfactor = 31;
    char sbuf[64] = {0};
    int n = snprintf(sbuf, sizeof(sbuf), "$2b$%02d$", workfactor);
    if (n <= 0 || (size_t)n >= sizeof(sbuf)) return -1;
    unsigned char *p = rnd;
    for (int i = 0; i < 16 && (size_t)n < sizeof(sbuf)-1; i += 3) {
        unsigned int v = 0;
        v = (unsigned int)p[i] << 16;
        if (i+1 < 16) v |= (unsigned int)p[i+1] << 8;
        if (i+2 < 16) v |= (unsigned int)p[i+2];
        for (int j = 0; j < 4 && (size_t)n < sizeof(sbuf)-1; ++j) {
            sbuf[n++] = bcrypt_base64_table[v & 0x3f];
            v >>= 6;
        }
    }
    sbuf[n] = '\0';
    strncpy(salt_out, sbuf, salt_out_len - 1);
    salt_out[salt_out_len - 1] = '\0';
    return 0;
#else
    if (workfactor < 4) workfactor = 4;
    if (workfactor > 31) workfactor = 31;
    unsigned char rnd[16];
    random_bytes(rnd, sizeof(rnd));
    char sbuf[64];
    int n = snprintf(sbuf, sizeof(sbuf), "$2b$%02d$", workfactor);
    int idx = n;
    for (int i = 0; i < 16 && idx < (int)sizeof(sbuf)-1; ++i) {
        sbuf[idx++] = bcrypt_base64_table[rnd[i] & 0x3f];
    }
    sbuf[idx] = '\0';
    strncpy(salt_out, sbuf, salt_out_len-1);
    salt_out[salt_out_len-1] = '\0';
    return 0;
#endif
}

int bcrypt_hashpw(const char *password, const char *salt, char *hash_out, size_t hash_out_len) {
    if (!password || !salt || !hash_out) return -1;
#ifdef HAVE_CRYPT
#if defined(_GNU_SOURCE)
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    char *res = crypt_r(password, salt, &data);
    if (!res) return -1;
    size_t needed = strlen(res) + 1;
    if (needed > hash_out_len) return -1;
    memcpy(hash_out, res, needed);
    return 0;
#else
    char *res = crypt(password, salt);
    if (!res) return -1;
    size_t needed = strlen(res) + 1;
    if (needed > hash_out_len) return -1;
    memcpy(hash_out, res, needed);
    return 0;
#endif
#else
    size_t plen = strlen(password);
    size_t slen = strlen(salt);
    unsigned int acc = 0x23456789u;
    for (size_t i = 0; i < plen; ++i) acc = acc * 101 + (unsigned char)password[i];
    for (size_t i = 0; i < slen; ++i) acc = acc * 113 + (unsigned char)salt[i];
    int written = snprintf(hash_out, hash_out_len, "$2x$%02d$%08x%08x%08x", 12, acc, acc ^ 0xABCDEF01u, acc ^ 0x12345678u);
    if (written <= 0) return -1;
    return 0;
#endif
}

int bcrypt_checkpw(const char *password, const char *hash) {
    if (!password || !hash) return -1;
#ifdef HAVE_CRYPT
    char *res = crypt(password, hash);
    if (!res) return -1;
    const unsigned char *a = (const unsigned char *)res;
    const unsigned char *b = (const unsigned char *)hash;
    size_t la = strlen((const char*)a);
    size_t lb = strlen((const char*)b);
    if (la != lb) return -1;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; ++i) diff |= a[i] ^ b[i];
    return diff == 0 ? 0 : 1;
#else
    char computed[128];
    if (bcrypt_hashpw(password, hash, computed, sizeof(computed)) != 0) return -1;
    return strcmp(computed, hash) == 0 ? 0 : 1;
#endif
}