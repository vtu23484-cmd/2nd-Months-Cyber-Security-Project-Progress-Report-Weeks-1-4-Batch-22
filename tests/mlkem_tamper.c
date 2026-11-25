#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);

    if (kem == NULL) {
        fprintf(stderr, "ERROR: ML-KEM-768 is unavailable.\n");
        return 1;
    }

    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *original_secret = malloc(kem->length_shared_secret);
    uint8_t *tampered_secret = malloc(kem->length_shared_secret);

    if (!public_key || !secret_key || !ciphertext ||
        !original_secret || !tampered_secret) {
        fprintf(stderr, "ERROR: Memory allocation failed.\n");
        goto error;
    }

    printf("Step 1: Generating ML-KEM-768 key pair...\n");

    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Key generation failed.\n");
        goto error;
    }

    printf("Step 2: Generating original ciphertext and shared secret...\n");

    if (OQS_KEM_encaps(kem, ciphertext, original_secret,
                       public_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Encapsulation failed.\n");
        goto error;
    }

    printf("Step 3: Modifying one byte of the ciphertext...\n");

    ciphertext[0] ^= 0x01;

    printf("Step 4: Decapsulating modified ciphertext...\n");

    if (OQS_KEM_decaps(kem, tampered_secret, ciphertext,
                       secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Decapsulation operation failed.\n");
        goto error;
    }

    printf("Step 5: Comparing the shared secrets...\n");

    if (memcmp(original_secret, tampered_secret,
               kem->length_shared_secret) != 0) {
        printf("\nSUCCESS: The shared secrets are different.\n");
        printf("ML-KEM-768 implicit rejection behaviour verified.\n");

        free(public_key);
        OQS_MEM_secure_free(secret_key, kem->length_secret_key);
        free(ciphertext);
        OQS_MEM_secure_free(original_secret, kem->length_shared_secret);
        OQS_MEM_secure_free(tampered_secret, kem->length_shared_secret);
        OQS_KEM_free(kem);
        return 0;
    }

    fprintf(stderr, "\nFAILED: The shared secrets unexpectedly match.\n");

error:
    free(public_key);
    OQS_MEM_secure_free(secret_key, kem->length_secret_key);
    free(ciphertext);
    OQS_MEM_secure_free(original_secret, kem->length_shared_secret);
    OQS_MEM_secure_free(tampered_secret, kem->length_shared_secret);
    OQS_KEM_free(kem);
    return 1;
}
