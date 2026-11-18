#ifndef PQC_ENGINE_H
#define PQC_ENGINE_H

#include <stddef.h>
#include <stdint.h>

int qv_generate_keypair(const char *pub_key_path, const char *sec_key_path);

int qv_encrypt_buffer(const uint8_t *plaintext, size_t plaintext_len,
                      const char *pub_key_path,
                      uint8_t **ciphertext_out, size_t *ciphertext_len_out);

void qv_free_buffer(uint8_t *buffer);

#endif // PQC_ENGINE_H
