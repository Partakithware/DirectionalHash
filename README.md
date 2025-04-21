# DirectionalHash (dhash)
**Author:** Maxwell Wingate  
**Version:** Release Candidate 1  
**License:** MIT  
**Platforms:** Linux, Windows, macOS (C Cross-Platform Compatible)

---

## Overview

**DirectionalHash (dhash)** is an advanced, grid-based preprocessing system layered over cryptographic digest functions. It introduces byte-level entropy transformation through a **weighted, positionally-biased 3x3 binary grid**, enabling highly sensitive structural interpretation of file content prior to digest generation. This architecture makes dhash particularly effective for **double-verification systems**, where traditional cryptographic hashes (e.g., SHA-256, SHA-512) are supplemented with a logic-driven, pattern-sensitive verification layer.

**DirectionalHash is not a replacement for cryptographic hash functions.** Instead, it enhances verification workflows by exposing subtle differences in binary structure that may not impact conventional hashes, offering unique value in digital forensics, integrity auditing, and secure archival environments.

---

## Key Differentiators

- ✅ **Bit-to-grid preprocessing:** Every byte is decomposed into a 3x3 grid with one randomized blank cell, producing spatial variation in structure.
- ✅ **Weighted directional reads:** Each grid undergoes entropy-weighted readouts based on grid population and positional bias, creating a dynamic output stream.
- ✅ **Permutation-driven output:** The final bitstream reflects not only byte values but their structural implication—making even minor binary differences detectable.
- ✅ **Cross-platform compatibility:** C-based implementation with OpenSSL and OpenMP ensures performance and portability across all major OS environments.

---

## Methodology

### 1. **Grid Encoding**
Each byte is converted to an 8-bit binary string and mapped left-to-right, top-down into a 3x3 grid. The final cell remains empty (`NULL`), introducing a variable node in each matrix.

### 2. **Weight Bias and Entropy**
Each grid cell is scored using a weight matrix influenced by:
- Binary value (`1` vs `0`)
- Positional influence (center vs edge vs corner)
These scores dictate traversal order and control how each byte is serialized into the hash pipeline.

### 3. **Flattening Process**
Grids are traversed in weight-descending order, not fixed patterns. This allows **adaptive reads per byte**, maximizing structural sensitivity.

### 4. **Bitstream Reassembly**
Serialized bits are re-packed into bytes, forming the intermediate data that is digested using a selected cryptographic hash (SHA256, SHA512, or SHAKE256 for extended output).

### 5. **Final Hash Output**
The final hash is the cryptographic digest of the entropy-transformed byte stream, highly sensitive to both value and bit-placement changes.

---

## Features

- ✔️ **Output sizes:** 256, 512, 1024, 2048 bits
- ✔️ **Threading:** Multi-core support via OpenMP
- ✔️ **Performance:** Optimized C implementation using chunk-based I/O
- ✔️ **Cross-platform:** Runs on Linux, macOS, Windows
- ✔️ **Optional timing:** Use `--time` to benchmark hash duration
- ✔️ **.deb package available** for easy installation on Debian-based systems

---

## Usage

```bash
# Default: 256-bit hash, 8192-byte chunks, 4 threads
./directional_hash myfile.txt

# Custom size, chunking, and threading
./directional_hash myfile.iso 1024 16384 8

# Include timing output
./directional_hash myfile.deb 512 8192 6 --time
