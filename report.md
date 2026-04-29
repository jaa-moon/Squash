# Squash Compression Tool - Comprehensive Project Report

## 1. Project Overview
**Squash** is a custom, lightweight, command-line data compression and decompression utility written in C++. It utilizes a sliding-window data compression algorithm based on the foundational **LZ77** (Lempel-Ziv) methodology. The project provides a complete end-to-end pipeline capable of compressing arbitrary files, decompressing them back to their original state, verifying data integrity via checksums, and reporting compression efficiency. 

This document serves as a comprehensive, deep-dive examination of the project's source code, component interactions, and technical design details.

## 2. System Architecture & File Structure
The project is modularized into four distinct C++ source files, keeping concerns cleanly separated, and a Makefile for simple compilation.

- `main.cpp`: Serves as the user-facing entry point. Parses command-line arguments and routes the user's request to the appropriate subsystem.
- `compress.cpp`: Contains the core compression logic, implementing the LZ77 sliding window pattern-matching and binary encoding.
- `decompress.cpp`: Contains the decompression logic, reversing the encoding by parsing literal bytes and back-references, along with integrity verification formatting.
- `stats.cpp`: A utility module that evaluates compression efficiency by comparing original and compressed file sizes.
- `Makefile`: Provides a standard, single-target build configuration for compiling the application using `g++`.

---

## 3. Detailed Component Analysis

### 3.1. `main.cpp` (The Entry Point)
The `main.cpp` file acts as the core controller. It declares the external functions (`compress`, `decompress`, `stats`) defined in the other files. 
- **Execution Flow**: It checks `argc` to ensure the correct number of arguments are provided (expecting 4: the executable name, the command, the input file, and the output file).
- **Routing**: It checks the first argument string (`argv[1]`) against known commands (`"compress"`, `"decompress"`, `"stats"`) and routes to the respective functions.
- **Help Utility**: If unexpected arguments are passed or no arguments are provided, it drops into a `help()` function to guide the user on proper CLI syntax.

### 3.2. `compress.cpp` (The Encoder)
This file implements the LZ77 algorithm. It consists of two major components: `findMatch()` and `compress()`.

#### `findMatch(vector<char> &data, int pos)`
This helper function searches backward from the current read position (`pos`) to find the longest sequence of bytes that matches the upcoming bytes.
- **The Window**: It looks back up to `WINDOW` (4096) bytes. The start of the search is dynamically computed as `max(pos - WINDOW, 0)`.
- **The Search**: It iterates through the history buffer. For each previous position `i`, it counts how many consecutive bytes match the bytes at `pos + length`.
- **Match Limitations**: It only matches up to `MAX_MATCH` (18 bytes) to fit the length metadata into a single byte during encoding.
- **Return Type**: It returns a `std::pair<int, int>` containing the `bestOffset` (how far back the match is) and `bestLength` (how many bytes match).

#### `compress(string inputPath, string outputPath)`
The main compression routine.
- **File I/O**: It loads the **entire** input file into memory as a `vector<char>`. This simplifies the algorithm but is a notable limitation for files larger than available RAM.
- **Checksum Calculation**: A simple additive checksum (the sum of every uncompressed byte value) is stored in a 32-bit `unsigned int` to be written later.
- **Encoding Loop (Groups of 8)**: To efficiently flag whether a token is a single uncompressed byte (literal) or a back-reference, Squash uses a **Flag Byte**. One Flag Byte contains 8 bits, meaning it dictates the type of the next 8 tokens.
- **Stream Positional Tracking**: Since the Flag Byte must precede the tokens, the program saves the current file pointer `flagPos = out.tellp()`, writes a dummy null-byte `\0`, encodes up to 8 tokens while modifying a local `flagByte` integer, and then rewrites the exact `flagByte` back at `flagPos`.
- **Token Formatting**:
  - **Match > 3 bytes**: It sets the respective bit in the `flagByte` to `1`. The match is encoded into 3 bytes: 2 bytes for the `offset` (cast to `unsigned short`) and 1 byte for the `length` (cast to `unsigned char`). It skips `pos` ahead by `length`. Matches strictly shorter than 3 bytes are ignored (because dedicating 3 bytes of metadata to save 1 or 2 bytes is counterproductive).
  - **Literal**: It leaves the respective bit as `0` and writes the uncompressed byte directly to the file. `pos` increments by 1.
- **Finalization**: At the very end of the file, the 4-byte checksum is appended.

### 3.3. `decompress.cpp` (The Decoder)
This module reconstructs original files by parsing the encoded instructions from `compress.cpp`.

- **File Parsing and Integrity**: Like compression, the file is read entirely into an `std::vector<char>`. The very last 4 bytes of this vector are immediately sliced off and interpreted as the `storedChecksum` using `memcpy`.
- **Decoding Loop**:
  - It iterates over the data (minus the last 4 bytes).
  - The first byte of every group is pulled as the `flagByte`.
  - A loop iterates 8 times to process the next 8 tokens according to the `flagByte` bits.
- **Handling Tokens**:
  - **Back-reference (bit == 1)**: The program reads the next 2 bytes as the `offset` and 1 byte as the `length`. It goes into its own decoded `output` buffer, calculates the `start = output.size() - offset`, and duplicates `length` bytes to the end of the `output` array.
  - **Literal (bit == 0)**: It pushes the single literal byte to the `output` vector.
- **Checksum Verification**: After building the output vector, the decompressor iterates through all reconstructed bytes and calculates its own checksum. It compares this against the `storedChecksum`.
- **Writing**: The vector is flushed to disk via `ofstream::write()`. The terminal explicitly informs the user if a checksum mismatch occurred, signaling potential data corruption.

### 3.4. `stats.cpp` (The Analytics Tool)
This utility calculates exactly how much disk space was saved.
- **File Sizing**: It leverages `ifstream` alongside the `ios::ate` (At The End) flag. Using `tellg()` immediately grabs the size of the file in bytes without needing to read the content into memory.
- **Efficiency Calculation**: A mathematical comparison is made: `(1.0 - (compressed / original)) * 100.0`. 
- **Output Handling**: Values are safely bounded (e.g., bypassing calculation if the original file size is 0). It also alerts the user if compression was negative (the target grew in size, which is common with heavily randomized files or already-compressed data like JPEGs).

### 3.5. `Makefile`
A standard compilation instruction set. Running `make` compiles all four `.cpp` files concurrently and outputs the binary `squash`. `make clean` deletes the executable.

---

## 4. Strengths & Limitations

### 4.1. Strengths
- **Simplicity**: No external dependencies beyond the standard C++ library mappings (STL).
- **Correctness First**: Checksum validation prevents users from trusting silently mangled data.
- **Fast Decompression**: As typical in LZ algorithms, decompression involves purely memory-copy operations, avoiding computationally heavy searches.

### 4.2. Current Technical Limitations
- **Memory Consumption**: Both compression and decompression invoke `istreambuf_iterator` into `vector<char>`, loading the entire file into available system RAM simultaneously. Trying to compress a 10 GB file will demand at least 10 GB of RAM. A streaming approach using chunked reading would fix this.
- **Sub-optimal Match Finding**: The `findMatch` sliding window loops through every single character with no string-hashing, skip-logic, or tree-based lookup. This leads to $O(N \times W)$ time complexity (where $N$ is file size and $W$ is the window size), meaning poor performance on large files.
- **End-of-File Overhead**: Since groups **must** process 8 iterations, if a file ends midway through a group, standard loop constraints apply. The file format is structured heavily around arbitrary loops that could break edge cases if metadata isn't carefully validated against bounds.
