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
