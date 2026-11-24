#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);

    if (kem == NULL) {
        fprintf(stderr, "ERROR: ML-KEM-768 is not available.\n");
        return 1;
    }

    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *sender_secret = malloc(kem->length_shared_secret);
    uint8_t *receiver_secret = malloc(kem->length_shared_secret);

    if (!public_key || !secret_key || !ciphertext ||
        !sender_secret || !receiver_secret) {
        fprintf(stderr, "ERROR: Memory allocation failed.\n");
        goto cleanup_error;
    }

    printf("Step 1: Generating ML-KEM-768 key pair...\n");

    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Key generation failed.\n");
        goto cleanup_error;
    }

    printf("Step 2: Encapsulating shared secret...\n");

    if (OQS_KEM_encaps(kem, ciphertext, sender_secret,
                       public_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Encapsulation failed.\n");
        goto cleanup_error;
    }

    printf("Step 3: Decapsulating shared secret...\n");

    if (OQS_KEM_decaps(kem, receiver_secret, ciphertext,
                       secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: Decapsulation failed.\n");
        goto cleanup_error;
    }

    printf("Step 4: Comparing both shared secrets...\n");

    if (memcmp(sender_secret, receiver_secret,
               kem->length_shared_secret) == 0) {
        printf("\nSUCCESS: Both shared secrets match!\n");
        printf("ML-KEM-768 encapsulation and decapsulation verified.\n");

        OQS_MEM_secure_free(secret_key, kem->length_secret_key);
        OQS_MEM_secure_free(sender_secret, kem->length_shared_secret);
        OQS_MEM_secure_free(receiver_secret, kem->length_shared_secret);
        free(public_key);
        free(ciphertext);
        OQS_KEM_free(kem);
        return 0;
    }

    fprintf(stderr, "\nFAILED: Shared secrets do not match.\n");

cleanup_error:
    free(public_key);
    OQS_MEM_secure_free(secret_key, kem->length_secret_key);
    free(ciphertext);
    OQS_MEM_secure_free(sender_secret, kem->length_shared_secret);
    OQS_MEM_secure_free(receiver_secret, kem->length_shared_secret);
    OQS_KEM_free(kem);
    return 1;
}
