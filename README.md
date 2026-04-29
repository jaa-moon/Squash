# Squash

**Squash** is a custom, lightweight, command-line data compression and decompression utility written in C++. It utilizes a sliding-window data compression algorithm based on the foundational **LZ77** (Lempel-Ziv) methodology. The project provides a complete end-to-end pipeline capable of compressing arbitrary files, decompressing them back to their original state, verifying data integrity via checksums, and reporting compression efficiency.

## Features

- **Compression**: Implements the LZ77 algorithm with a sliding window to compress files.
- **Decompression**: Fast decompression using purely memory-copy operations.
- **Integrity Verification**: Ensures data is not corrupted by validating an additive checksum upon decompression.
- **Statistics**: Provides tools to check compression efficiency and ratio.
- **Zero Dependencies**: Relies solely on the standard C++ library (STL).

## Architecture

The project is modularized into cleanly separated C++ source files:

- `main.cpp`: The user-facing entry point. Parses command-line arguments and routes requests.
- `compress.cpp`: Contains the core compression logic, implementing the LZ77 sliding window pattern-matching and binary encoding.
- `decompress.cpp`: Contains the decompression logic, reversing the encoding by parsing literal bytes and back-references, along with integrity verification formatting.
- `stats.cpp`: A utility module that evaluates compression efficiency by comparing original and compressed file sizes.
- `Makefile`: Provides a standard build configuration for compiling the application.

## Building the Project

Use the provided `Makefile` to compile the project:

```bash
make
```

To clean up the compiled binary:

```bash
make clean
```

## Usage

The `squash` executable expects the following command-line format:

```bash
./squash <command> <input_file> <output_file>
```

### Commands

- **`compress`**: Compresses the `input_file` and writes the result to `output_file`.
- **`decompress`**: Decompresses the `input_file` and writes the original content to `output_file`. Also verifies the checksum.
- **`stats`**: Compares `input_file` (original) and `output_file` (compressed) to display the compression ratio and space saved.

### Examples

**Compressing a file:**
```bash
./squash compress document.txt document.lz
```

**Decompressing a file:**
```bash
./squash decompress document.lz restored_document.txt
```

**Viewing statistics:**
```bash
./squash stats document.txt document.lz
```

## Technical Details

- **Algorithm**: LZ77 (Lempel-Ziv)
- **Window Size**: 4096 bytes
- **Max Match Length**: 18 bytes
- **Encoding**: Uses a "Flag Byte" to efficiently encode groups of 8 tokens (literals or back-references). Back-references take 3 bytes (2 for offset, 1 for length).

### Current Limitations

- **Memory Consumption**: Entire files are loaded into RAM simultaneously during compression and decompression.
- **Search Efficiency**: The sliding window uses an $O(N \times W)$ search strategy without string hashing.
