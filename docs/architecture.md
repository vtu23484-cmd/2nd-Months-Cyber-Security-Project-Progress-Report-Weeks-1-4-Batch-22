# QuantumVault System Architecture & Threat Model

## 1. Problem Statement & Threat Model
Classical public-key cryptography (such as RSA-2048 and ECC) relies on mathematical problems that Shor's algorithm on a sufficiently powerful quantum computer can break in polynomial time. Sensitive local files encrypted today are vulnerable to **"Harvest Now, Decrypt Later" (HNDL)** attacks, where adversaries capture encrypted data now and decrypt it once quantum hardware matures.

## 2. Hybrid Cryptographic Design
Pure Post-Quantum Cryptography (PQC) algorithms have larger key sizes and slower execution times for bulk data compared to symmetric ciphers. QuantumVault solves this using a **Hybrid Encryption Architecture**:

1. **Key Encapsulation Mechanism (KEM): ML-KEM-768 (NIST FIPS 203 / CRYSTALS-Kyber)**
   - Generates a quantum-resistant public key (`ek`) and private key (`dk`).
   - Encapsulates a 256-bit shared secret (`ss`) and produces a ciphertext (`ct`).
   - Provides NIST Security Level 3 protection (roughly equivalent to AES-192 against quantum attacks).
2. **Symmetric File Encryption: AES-256-GCM**
   - Uses the 32-byte shared secret derived from ML-KEM-768 as the symmetric encryption key.
   - Encrypts local files quickly while providing built-in **Authenticated Encryption with Associated Data (AEAD)** to detect any unauthorized file tampering.

## 3. Data Flow (Locking & Unlocking Files)

### Locking (Encryption) Workflow
```text
[Plaintext File] 
       │
       ▼
[ML-KEM-768 Public Key] ──► Encapsulate ──► [32-Byte Shared Secret] + [PQC Ciphertext]
                                                    │                       │
                                                    ▼                       │
                                           [AES-256-GCM Encrypt]            │
                                                    │                       │
                                                    ▼                       ▼
                                         [Encrypted File Data] + [PQC Header] ──► (.qvault file)
host
