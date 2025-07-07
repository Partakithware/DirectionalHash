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


CONS: Overhead/Time-Consumption: My test (limited machine - older laptop) 

15-seconds fastest recorded time per 170mb (rc5 test, not great for big files but its intended for increased security checks not speed)

my@linuxpc:~/Documents/Dhash$ ./dhash "/home/username/Downloads/Vortex-1-1-14-3-1750157786.exe" 512 4096 4 --time
d68732b0a195e6b9310bb1a492932b5eae1f45f2d40e8e5c35e38f7f5f536cdfc7cab666d48b2d4c2d8d6d3103b8d60913878a2b73bf26402f33cf94f264aecf
Time elapsed: 15.271838 seconds


0.696456 seconds or less for 4mb files (can set various things to increase speeds)

27mb file test:

my@linuxpc:~/Documents/Dhash$ ./dhash "/home/username/Downloads/PKMN.NETSR4.zip" 512 8192 4 --time
e3e9ef6a78c35207fc00d78887cb24b62df1547ab1869cbef71bcdb0715d73cc9c8eeafce2f49b4430148b76e1ddc6e6ec0d3d5021da58783d2e067ba8979f72
Time elapsed: 2.337796 seconds


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
