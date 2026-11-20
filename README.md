# QuantumVault — Post-Quantum Encrypted Local Filesystem

QuantumVault is an experimental Linux filesystem project exploring post-quantum cryptography and transparent file encryption. It combines a C-based cryptographic engine using Open Quantum Safe (`liboqs`) with a Rust-based FUSE filesystem to intercept file writes and store encrypted data in a separate backing directory.

The project explores protection against the **“Harvest Now, Decrypt Later”** threat model, where encrypted information may be collected today with the intention of decrypting it in the future as cryptographic capabilities evolve.

> **Project status:** Experimental prototype. Encryption on write is implemented, but authenticated encryption and transparent decryption on read are not yet complete. This project is not suitable for protecting real sensitive data.

---

## Features Implemented

- Post-quantum key generation using ML-KEM-768 (formerly CRYSTALS-Kyber).
- C cryptographic engine built using `liboqs`.
- Rust-to-C FFI integration.
- FUSE-based virtual filesystem.
- Write interception through the FUSE filesystem.
- Encrypted backing files stored with the `.qvault` extension.
- Automatic key-file generation during initial startup.
- Built-in `hello.txt` diagnostic file.

## Technologies Used

| Technology | Purpose |
|---|---|
| C | Cryptographic engine |
| Rust | Filesystem implementation |
| FUSE 3 | Userspace filesystem interface |
| liboqs | Post-quantum cryptographic library |
| ML-KEM-768 | Key encapsulation mechanism |
| Make | Build automation |
| Ubuntu/Linux | Development and execution environment |

---

# Setup, Build & Run Guide

## 1. Operating System

The project is intended to be built and run on Ubuntu Linux.

You can use:
- Ubuntu installed directly
- Ubuntu inside a virtual machine

A Linux environment with working FUSE support is required.

## 2. Install Required Packages

Open a terminal and install the build dependencies:

```bash
sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    libssl-dev \
    libfuse3-dev \
    fuse3 \
    pkg-config \
    git \
    curl \
    xxd
```

## 3. Install Rust

Install Rust using `rustup`:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
```

Load Rust into the current terminal:

```bash
source "$HOME/.cargo/env"
```

Verify installation:

```bash
rustc --version
cargo --version
```

## 4. Install liboqs

Clone the Open Quantum Safe `liboqs` repository:

```bash
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git /tmp/liboqs
```

Configure the build:

```bash
cmake -S /tmp/liboqs \
      -B /tmp/liboqs/build \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX=/usr/local
```

Compile the library:

```bash
cmake --build /tmp/liboqs/build --parallel
```

Install it:

```bash
sudo cmake --install /tmp/liboqs/build
```

Refresh the shared library cache:

```bash
sudo ldconfig
```

## 5. Clone QuantumVault

Clone the project repository:

```bash
git clone https://github.com/vtu23484-cmd/2nd-Months-Cyber-Security-Project-Progress-Report-Weeks-1-4-Batch-22.git
```

Move into the project directory:

```bash
cd 2nd-Months-Cyber-Security-Project-Progress-Report-Weeks-1-4-Batch-22
```

## 6. Build the Project

QuantumVault includes a Makefile to automate compilation.

Compile the C shared library and build the Rust FUSE application:

```bash
make
```

The Makefile performs the following steps:

1. Creates the `build/` directory.
2. Compiles `src/pqc_engine.c` into `build/libpqc_engine.so`.
3. Links the C cryptographic engine with `liboqs`.
4. Builds the Rust FUSE application using Cargo.

The expected compiled library is:

```text
build/libpqc_engine.so
```

## 7. Run and Mount QuantumVault

Create the mount directory:

```bash
mkdir -p /tmp/qvault_mount
```

Start QuantumVault using:

```bash
make mount
```

This command builds the project if required and launches the FUSE filesystem at:

```text
/tmp/qvault_mount
```

Keep this terminal running while the filesystem is mounted.

On the initial launch, QuantumVault attempts to generate its ML-KEM-768 keypair and store it in the `vault_keys/` directory.

---

# Demonstration & Verification

## 8. Open a Second Terminal

Keep the mounting terminal running.

Open a second terminal and move into the project directory:

```bash
cd 2nd-Months-Cyber-Security-Project-Progress-Report-Weeks-1-4-Batch-22
```

## 9. Test the Virtual Filesystem

Read the built-in diagnostic file:

```bash
cat /tmp/qvault_mount/hello.txt
```

Expected output:

```text
Hello World from QuantumVault FUSE!
```

## 10. Write a Test File

Write a sample message into the mounted filesystem:

```bash
echo "Top Secret Blueprints - QuantumVault 2026" > /tmp/qvault_mount/blueprint.txt
```

The FUSE write callback intercepts the write and invokes the C encryption engine.

## 11. Verify the Backing Storage

List the encrypted storage directory:

```bash
ls -lh vault_storage/
```

Look for the corresponding encrypted file:

```text
blueprint.txt.qvault
```

Inspect the file contents in hexadecimal format:

```bash
xxd vault_storage/blueprint.txt.qvault | head -n 10
```

The backing file should contain binary data rather than the original plaintext text.

## 12. Verify Key Files

Check whether the key files were generated:

```bash
ls -lh vault_keys/
```

Expected files:

```text
public.key
secret.key
```

**Never upload your private key (`secret.key`) to a public repository.**

---

# Unmounting QuantumVault

When you finish testing, return to your second terminal and execute:

```bash
make unmount
```

Alternatively:

```bash
fusermount3 -u /tmp/qvault_mount
```

Verify that the filesystem is no longer mounted:

```bash
mount | grep qvault_mount
```

If there is no output, the mount is no longer listed.

---

# Project Directory Structure

The major components of the repository are organized as follows:

```text
QuantumVault/
├── docs/
│   └── Project documentation
├── src/
│   ├── pqc_engine.c
│   ├── pqc_engine.h
│   └── quantumvault_fuse/
│       ├── Cargo.toml
│       ├── build.rs
│       └── src/
│           ├── main.rs
│           └── ffi.rs
├── build/
│   └── libpqc_engine.so
├── vault_keys/
│   ├── public.key
│   └── secret.key
├── vault_storage/
│   └── *.qvault
├── Makefile
└── README.md
```

Some directories such as `build/`, `vault_keys/`, and `vault_storage/` are created during execution and may not exist in a fresh clone.

---

# Makefile Commands

| Command | Description |
|---|---|
| `make` | Builds the C shared library and Rust application |
| `make fuse` | Builds the cryptographic library and FUSE application |
| `make mount` | Builds and mounts QuantumVault |
| `make unmount` | Unmounts the filesystem |
| `make clean` | Removes generated build artifacts |

---

# Current Limitations

QuantumVault is an ongoing academic prototype. The following capabilities are not yet complete:

- Transparent decryption when reading stored files.
- Proper authenticated symmetric encryption of file contents.
- Secure handling of repeated, partial, and offset-based file writes.
- Comprehensive error handling and filesystem persistence testing.
- Production-ready key management and secure key storage.

The current ML-KEM implementation establishes a shared secret, but the file-data XOR construction is experimental and is not a secure replacement for an authenticated encryption algorithm. The project should not be used to store confidential or sensitive files in its current state.

---

# WEEK 1 – IMPLEMENTATION REPORT

## 1. Introduction

QuantumVault is designed to protect sensitive local files against "Harvest Now, Decrypt Later" quantum computing threats through transparent, quantum-resistant encryption. During Week 1, the Linux development environment was prepared to build the project's two foundational layers: a post-quantum cryptographic engine powered by C and `liboqs` (CRYSTALS-Kyber), and a userspace virtual filesystem interface built with Rust and FUSE.

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

During Week 2 of the QuantumVault project, the standalone cryptographic engine and virtual filesystem built in Week 1 were integrated into a unified pipeline. The primary objective was to build a Rust-to-C Foreign Function Interface (FFI) bridge and hook into Linux FUSE `create()` and `write()` callbacks to perform transparent, on-the-fly CRYSTALS-Kyber (ML-KEM-768) encryption whenever files are saved to the virtual mount.

## 2. Work Completed

### 2.1 C Shared Library & Rust FFI Bridge

- Refactored the PQC logic into `src/pqc_engine.h` and `src/pqc_engine.c`, compiled into a position-independent Linux shared library (`build/libpqc_engine.so`).
- Implemented `src/quantumvault_fuse/src/ffi.rs` and `build.rs` using `extern "C"` bindings to safely invoke `qv_generate_keypair()` and `qv_encrypt_buffer()` from Rust with explicit C buffer cleanup (`qv_free_buffer()`).

### 2.2 Transparent Encryption on FUSE `write()`

- Upgraded `QuantumVaultFS` from Read-Only to Read/Write mode (`MountOption::RW`).
- Added automatic master keypair generation (`vault_keys/public.key` and `vault_keys/secret.key`) at mount initialization.
- Hooked `create()`, `open()`, `setattr()`, and `write()` in `src/quantumvault_fuse/src/main.rs` so plaintext written to `/tmp/qvault_mount/<filename>` is intercepted, encrypted with ML-KEM-768, and stored on disk as `vault_storage/<filename>.qvault`.

## 3. Verification & Mid-Project Audit

- Verified end-to-end transparent encryption by writing `blueprint.txt` into `/tmp/qvault_mount` and inspecting `vault_storage/blueprint.txt.qvault` via `xxd` (confirming 1,088 bytes of Kyber-768 KEM ciphertext plus encrypted payload bytes).
- Verified with `openssl enc -aes-256-cbc -d` that classical decryption utilities fail (`bad magic number`) against the quantum-encrypted `.qvault` files.
