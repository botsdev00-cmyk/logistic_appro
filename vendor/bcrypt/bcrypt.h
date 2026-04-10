/*
 * Minimal header for vendorized bcrypt implementation.
 *
 * Public functions:
 *   int bcrypt_gensalt(int workfactor, char *salt_out, size_t salt_out_len);
 *   int bcrypt_hashpw(const char *password, const char *salt, char *hash_out, size_t hash_out_len);
 *   int bcrypt_checkpw(const char *password, const char *hash);
 */

#ifndef VENDOR_BCRYPT_H
#define VENDOR_BCRYPT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int bcrypt_gensalt(int workfactor, char *salt_out, size_t salt_out_len);
int bcrypt_hashpw(const char *password, const char *salt, char *hash_out, size_t hash_out_len);
int bcrypt_checkpw(const char *password, const char *hash);

#ifdef __cplusplus
}
#endif

#endif /* VENDOR_BCRYPT_H */