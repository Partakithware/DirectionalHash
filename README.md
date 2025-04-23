# DirectionalHash (`dhash`)

**Author:** Maxwell Wingate  
**Version:** Release Candidate 4  
**License:** MIT  
**Platforms:** Linux, Windows, macOS (C Cross-Platform Compatible)

**Version-Depicted:** RC4
---

## 🧭 Overview

**DirectionalHash (dhash)** is an advanced, grid-based preprocessing system layered over cryptographic digest functions. It transforms file content by processing byte-level data through a weighted, positionally-biased 3×3 binary grid.

This process enhances the sensitivity of the resulting hash, making it especially useful for **double-verification systems** where conventional hashes (e.g., SHA-256, SHA-512) are supplemented with a **logic-driven, pattern-sensitive verification layer**.

> DirectionalHash is **not** a replacement for cryptographic hash functions. Instead, it augments them—ideal for **digital forensics**, **integrity auditing**, and **secure archival environments**.


CONS: Overhead/Time-Consumption: My test (limited machine - older laptop) 20-seconds fastest recorded time per 150mb (not great for big files but its intended for increased security checks not speed)
---

## ✨ Key Differentiators

✅ **Bit-to-grid preprocessing**  
→ Every byte is decomposed into an 8-bit binary string and mapped into a 3×3 grid (1 empty cell).

✅ **Weighted directional reads**  
→ Grid cells receive weights based on binary value and positional bias.

✅ **Permutation-driven output**  
→ Final bitstream order is based on descending weights, incorporating both value and structure.

✅ **Cross-platform compatibility**  
→ Implemented in C using OpenSSL and OpenMP—optimized for speed and portability.

---

## ⚙️ Methodology

### 🧩 Grid Encoding
- Convert each byte into an 8-bit binary string.
- Map bits into a 3×3 grid (left-to-right, top-down).
- Leave the final cell empty (`NULL`).

### 🧮 Weight and Bias
Each cell’s weight is based on:
- Its **binary value** (`1` > `0`)
- Its **position** (center, edge, corner) — introduces spatial bias

### 🎲 Coordinate Shuffling
- A **seed** is generated for each byte using surrounding byte values.
- This seed determines the coordinate traversal order.

### 🧵 Flattening Process
- Traverse the grid in **descending weight order**, using the shuffled coordinates.
- Serialize the result into a bitstream.

### 🧱 Bitstream Reassembly
- Bits are packed into bytes.

### 🔐 Final Hash Output
- The byte stream is passed through a cryptographic hash (SHA-256, SHA-512, or SHAKE256).
- The result is **highly sensitive** to both data content and structure.

---

## 🚀 Features

✔️ Output sizes: `256`, `512`, `1024`, `2048` bits  
✔️ Multi-threading: Via OpenMP  
✔️ High-performance: Chunk-based I/O in C  
✔️ Cross-platform: Linux, macOS, Windows  
✔️ Optional benchmarking: Use `--time` to time hash duration  
✔️ `.deb` installer for Debian-based systems  

---

## 🛠️ Usage

```bash
# Default: 256-bit hash, 512-byte chunks, 4 threads (.deb release)
dhash myfile.txt

# Custom: 1024-bit hash, 8192-byte chunks, 8 threads
dhash myfile.iso 1024 8192 8

# Include timing output
dhash myfile.deb 512 8192 6 --time
