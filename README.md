# DirectionalHash (dhash)
Author: Maxwell Wingate
Version: Release Candidate 1
License: MIT
Platforms: Linux, Windows, macOS (C Cross-Platform Compatible)

# Overview
DirectionalHash (dhash) is a novel, permutation-based hashing algorithm designed to provide a secondary verification mechanism alongside standard cryptographic hashes such as SHA-256 or SHA-512. Where traditional hash functions rely on mathematical transformations, dhash introduces spatial permutations using a 3x3 binary grid transformation technique, allowing bit-level directional analysis of data. This makes it particularly valuable for double-verification scenarios, where increased sensitivity to low-level binary changes is desirable.

This algorithm is not intended to replace secure cryptographic hash functions but to supplement them in applications involving file integrity, forensic verification, or high-security checksum validation workflows.

# Motivation
Standard hashing algorithms like SHA-256 are optimized for speed and cryptographic security. However, in certain edge cases involving highly structured or transformed data, a second, differently-behaving hashing layer can detect anomalies that might otherwise be overlooked.

DirectionalHash addresses this by introducing directional permutations at the bit level, capturing both structural and sequential information via multiple read paths across a compact 3x3 grid.

# How It Works
# Bit-to-Grid Mapping:
Each byte is converted into an 8-bit binary string and mapped to a 3x3 grid. The ninth cell remains blank, introducing positional variability.

# Directional Reads:
The grid is read in four distinct, predefined directional permutations. These reads are:

Left-to-right, top-down

Right-to-left, bottom-up

Diagonally (TL-BR and TR-BL)

Mixed pattern with center-first traversal

# Flattening and Concatenation:
The values from each read permutation are flattened into a continuous string of bits.

# Repacking and Hashing:
The bitstring is repacked into bytes and fed into a cryptographic digest (SHA256, SHA512, SHAKE256 depending on configuration) to produce the final hash.

# Features
✅ 256, 512, 1024, and 2048-bit output support

✅ Written in portable C with OpenSSL dependency

✅ Chunk-based reading for memory efficiency

✅ Optional parallelism hooks (OpenMP-ready)

✅ .deb package available for Debian-based systems

# Use Cases
Redundant Verification
Use dhash in tandem with SHA-256 to detect binary-level modifications that may evade pattern-agnostic hashing.

Forensic Applications
Leverage directional sensitivity to detect non-obvious file manipulations or tampering.

Custom Security Systems
Define unique grid read sequences per organization/site to further obfuscate and protect against hash precomputation attacks.

# Example
# Basic usage with default settings
dhash myfile.zip
# 1024-bit hash with 8KB chunks and 4 worker threads
dhash myfile.iso 1024 8192 4


# Performance Note
DirectionalHash prioritizes thoroughness over speed. A 150MB file may take several minutes to verify depending on system specifications, whereas lightweight files (under 1MB) are processed in under a second. It is particularly optimized for batch verification of many small files.

# Future Goals
Parallelization improvements (OpenMP tuning)

Integration with GUI file integrity checkers

Customizable directional pattern sets

Language bindings for Python, Rust, and Go

# Disclaimer
DirectionalHash is not intended to replace cryptographic standards. It should be used as a supplementary hash function in scenarios where detailed binary structure analysis is necessary. It is ideal for enhancing integrity verification—not for standalone encryption or password storage.

# Acknowledgments
This project is the original work of Maxwell Wingate, who conceptualized and directed the design of the algorithm. It was brought to life with support from OpenAI assistance and hopefully made useful by community testing and potential updates.



