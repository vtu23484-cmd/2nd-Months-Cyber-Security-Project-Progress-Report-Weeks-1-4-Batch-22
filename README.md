# WEEK 1 – IMPLEMENTATION REPORT

## 1. Introduction
**QuantumVault** is designed to protect sensitive local files against "Harvest Now, Decrypt Later" quantum computing threats through transparent, quantum-resistant encryption. During Week 1, the Linux development environment was prepared to build the project's two foundational layers: a post-quantum cryptographic engine powered by **C and `liboqs`** (CRYSTALS-Kyber), and a userspace virtual filesystem interface built with **Rust and FUSE**.

## 2. Work Completed

### 2.1 Post-Quantum Cryptography Setup (C and liboqs)
The initial work focused on setting up and compiling the Open Quantum Safe (`liboqs`) library. A basic C program was developed to understand the working of post-quantum cryptographic operations.

The program was designed to generate a CRYSTALS-Kyber public and private key pair. It also performed encryption and decryption operations on a short message to verify the basic functionality of the cryptographic implementation.

### 2.2 Rust and FUSE Filesystem Setup
The second part involved setting up a Rust development environment and creating the initial project structure for filesystem integration.

A basic FUSE (Filesystem in Userspace) interface was implemented to understand how a virtual filesystem works. The application was designed to mount a virtual directory and return the message `"Hello World"` whenever a file was accessed and read.

## 3. Technologies Used
- C Programming Language
- Rust Programming Language
- Open Quantum Safe (`liboqs`)
- CRYSTALS-Kyber
- FUSE (Filesystem in Userspace)
- Linux Operating System

## 4. Learning Outcomes
- Understood the fundamentals of post-quantum cryptography.
- Learned the purpose and basic usage of the `liboqs` library.
- Understood public-key and private-key generation.
- Explored basic encryption and decryption operations.
- Learned the fundamentals of Rust project development.
- Understood how FUSE enables the creation of virtual filesystems.
- Gained practical knowledge of mounting and accessing a virtual directory.

## 5. Conclusion
The first week established the basic foundation for the QuantumVault project. The cryptographic component introduced post-quantum key generation and encryption operations using C and `liboqs`, while the filesystem component introduced virtual directory creation using Rust and FUSE. These initial implementations provide the foundation for integrating transparent encryption and decryption into the virtual filesystem in the upcoming weeks.

---

# WEEK 2 – IMPLEMENTATION REPORT

## 1. Introduction
During Week 2 of the **QuantumVault** project, the standalone cryptographic engine and virtual filesystem built in Week 1 were integrated into a unified pipeline. The primary objective was to build a **Rust-to-C Foreign Function Interface (FFI)** bridge and hook into Linux FUSE `create()` and `write()` callbacks to perform transparent, on-the-fly **CRYSTALS-Kyber (ML-KEM-768)** encryption whenever files are saved to the virtual mount.

## 2. Work Completed

### 2.1 C Shared Library & Rust FFI Bridge
- Refactored the PQC logic into `src/pqc_engine.h` and `src/pqc_engine.c`, compiled into a position-independent Linux shared library (`build/libpqc_engine.so`).
- Implemented `src/quantumvault_fuse/src/ffi.rs` and `build.rs` using `extern "C"` bindings to safely invoke `qv_generate_keypair()` and `qv_encrypt_buffer()` from Rust with explicit C buffer cleanup (`qv_free_buffer()`).

### 2.2 Transparent Encryption on FUSE `write()`
- Upgraded `QuantumVaultFS` from Read-Only to **Read/Write** mode (`MountOption::RW`).
- Added automatic master keypair generation (`vault_keys/public.key` and `vault_keys/secret.key`) at mount initialization.
- Hooked `create()`, `open()`, `setattr()`, and `write()` in `src/quantumvault_fuse/src/main.rs` so plaintext written to `/tmp/qvault_mount/<filename>` is intercepted, encrypted with ML-KEM-768, and stored on disk as `vault_storage/<filename>.qvault`.

## 3. Verification & Mid-Project Audit
- Verified end-to-end transparent encryption by writing `blueprint.txt` into `/tmp/qvault_mount` and inspecting `vault_storage/blueprint.txt.qvault` via `xxd` (confirming 1,088 bytes of Kyber-768 KEM ciphertext plus encrypted payload bytes).
- Verified with `openssl enc -aes-256-cbc -d` that classical decryption utilities fail (`bad magic number`) against the quantum-encrypted `.qvault` files.
