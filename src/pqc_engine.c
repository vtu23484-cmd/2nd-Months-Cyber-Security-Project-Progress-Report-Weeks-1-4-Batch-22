#include "pqc_engine.h"
#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int qv_generate_keypair(const char *pub_key_path, const char *sec_key_path) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);
    if (!kem) return -1;

    uint8_t *pk = malloc(kem->length_public_key);
    uint8_t *sk = malloc(kem->length_secret_key);
    if (!pk || !sk) {
        free(pk);
        free(sk);
        OQS_KEM_free(kem);
        return -2;
    }

    if (OQS_KEM_keypair(kem, pk, sk) != OQS_SUCCESS) {
        free(pk);
        OQS_MEM_secure_free(sk, kem->length_secret_key);
        OQS_KEM_free(kem);
        return -3;
    }

    FILE *f_pub = fopen(pub_key_path, "wb");
    FILE *f_sec = fopen(sec_key_path, "wb");
    if (!f_pub || !f_sec) {
        if (f_pub) fclose(f_pub);
        if (f_sec) fclose(f_sec);
        free(pk);
        OQS_MEM_secure_free(sk, kem->length_secret_key);
        OQS_KEM_free(kem);
        return -4;
    }

    fwrite(pk, 1, kem->length_public_key, f_pub);
    fwrite(sk, 1, kem->length_secret_key, f_sec);
    fclose(f_pub);
    fclose(f_sec);

    free(pk);
    OQS_MEM_secure_free(sk, kem->length_secret_key);
    OQS_KEM_free(kem);
    return 0;
}

int qv_encrypt_buffer(const uint8_t *plaintext, size_t plaintext_len,
                      const char *pub_key_path,
                      uint8_t **ciphertext_out, size_t *ciphertext_len_out) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);
    if (!kem) return -1;

    uint8_t *pk = malloc(kem->length_public_key);
    uint8_t *kem_ct = malloc(kem->length_ciphertext);
    uint8_t *shared_secret = malloc(kem->length_shared_secret);

    FILE *f_pub = fopen(pub_key_path, "rb");
    if (!f_pub || !pk || !kem_ct || !shared_secret) {
        if (f_pub) fclose(f_pub);
        free(pk);
        free(kem_ct);
        free(shared_secret);
        OQS_KEM_free(kem);
        return -2;
    }

    if (fread(pk, 1, kem->length_public_key, f_pub) != kem->length_public_key) {
        fclose(f_pub);
        free(pk);
        free(kem_ct);
        free(shared_secret);
        OQS_KEM_free(kem);
        return -3;
    }
    fclose(f_pub);

    if (OQS_KEM_encaps(kem, kem_ct, shared_secret, pk) != OQS_SUCCESS) {
        free(pk);
        free(kem_ct);
        OQS_MEM_secure_free(shared_secret, kem->length_shared_secret);
        OQS_KEM_free(kem);
        return -4;
    }

    // Output format: [KEM Ciphertext (1088 bytes)] + [Encrypted Payload (plaintext_len bytes)]
    size_t total_len = kem->length_ciphertext + plaintext_len;
    uint8_t *out = malloc(total_len);
    if (!out) {
        free(pk);
        free(kem_ct);
        OQS_MEM_secure_free(shared_secret, kem->length_shared_secret);
        OQS_KEM_free(kem);
        return -5;
    }

    memcpy(out, kem_ct, kem->length_ciphertext);
    for (size_t i = 0; i < plaintext_len; i++) {
        out[kem->length_ciphertext + i] = plaintext[i] ^ shared_secret[i % kem->length_shared_secret];
    }

    *ciphertext_out = out;
    *ciphertext_len_out = total_len;

    free(pk);
    free(kem_ct);
    OQS_MEM_secure_free(shared_secret, kem->length_shared_secret);
    OQS_KEM_free(kem);
    return 0;
}

void qv_free_buffer(uint8_t *buffer) {
    free(buffer);
}
