#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void xor_crypt(const uint8_t *input, uint8_t *output, size_t len, const uint8_t *key, size_t key_len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_len];
    }
}

int main(void) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);
    if (kem == NULL) {
        fprintf(stderr, "Error: ML-KEM-768 / Kyber-768 is not enabled in liboqs.\n");
        return EXIT_FAILURE;
    }

    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *shared_secret_sender = malloc(kem->length_shared_secret);
    uint8_t *shared_secret_receiver = malloc(kem->length_shared_secret);

    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "Error: Keypair generation failed.\n");
        return EXIT_FAILURE;
    }
    printf("[+] CRYSTALS-Kyber (ML-KEM-768) Keypair generated!\n");
    printf("    Public Key size: %zu bytes | Secret Key size: %zu bytes\n",
           kem->length_public_key, kem->length_secret_key);

    if (OQS_KEM_encaps(kem, ciphertext, shared_secret_sender, public_key) != OQS_SUCCESS) {
        fprintf(stderr, "Error: Encapsulation failed.\n");
        return EXIT_FAILURE;
    }

    if (OQS_KEM_decaps(kem, shared_secret_receiver, ciphertext, secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "Error: Decapsulation failed.\n");
        return EXIT_FAILURE;
    }

    if (memcmp(shared_secret_sender, shared_secret_receiver, kem->length_shared_secret) == 0) {
        printf("[+] Shared Secret verified successfully (%zu bytes)!\n", kem->length_shared_secret);
    } else {
        fprintf(stderr, "[-] Shared Secrets do not match!\n");
        return EXIT_FAILURE;
    }

    const char *message = "Hello QuantumVault!";
    size_t msg_len = strlen(message);
    uint8_t encrypted[64] = {0};
    char decrypted[64] = {0};

    xor_crypt((const uint8_t *)message, encrypted, msg_len, shared_secret_sender, kem->length_shared_secret);
    printf("[+] Original String : %s\n", message);
    printf("[+] Encrypted (Hex) : ");
    for (size_t i = 0; i < msg_len; i++) {
        printf("%02x", encrypted[i]);
    }
    printf("\n");

    xor_crypt(encrypted, (uint8_t *)decrypted, msg_len, shared_secret_receiver, kem->length_shared_secret);
    decrypted[msg_len] = '\0';
    printf("[+] Decrypted String: %s\n", decrypted);

    OQS_MEM_secure_free(secret_key, kem->length_secret_key);
    OQS_MEM_secure_free(shared_secret_sender, kem->length_shared_secret);
    OQS_MEM_secure_free(shared_secret_receiver, kem->length_shared_secret);
    free(public_key);
    free(ciphertext);
    OQS_KEM_free(kem);

    return EXIT_SUCCESS;
}
