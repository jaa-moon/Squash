#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

bool decompress(string inputPath, string outputPath) {
  // Read entire compressed file
  ifstream in(inputPath, ios::binary);
  if (!in) {
    cout << "Error: cannot open " << inputPath << endl;
    return false;
  }
  vector<char> compressed((istreambuf_iterator<char>(in)),
                          istreambuf_iterator<char>());
  in.close();

  // Last 4 bytes are the checksum
  if (compressed.size() < 4) {
    cout << "Error: file too small to be valid" << endl;
    return false;
  }
  unsigned int storedChecksum;
  memcpy(&storedChecksum, &compressed[compressed.size() - 4], 4);

  // Decode everything except the last 4 bytes (checksum)
  vector<char> output;
  int pos = 0;
  int end = (int)compressed.size() - 4;

  // Decode groups of 8 tokens, each group starts with a flag byte
  while (pos < end) {
    unsigned char flagByte = (unsigned char)compressed[pos];
    pos++;

    for (int bit = 0; bit < 8 && pos < end; bit++) {
      if (flagByte & (1 << bit)) {
        // Back-reference: read offset (2 bytes) and length (1 byte)
        if (pos + 3 > end)
          break;
        unsigned short offset;
        memcpy(&offset, &compressed[pos], 2);
        pos += 2;
        unsigned char length = (unsigned char)compressed[pos];
        pos++;

        // Copy from already-decoded output
        int start = (int)output.size() - offset;
        for (int i = 0; i < length; i++) {
          output.push_back(output[start + i]);
        }
      } else {
        // Literal byte
        output.push_back(compressed[pos]);
        pos++;
      }
    }
  }

  // Verify checksum
  unsigned int checksum = 0;
  for (unsigned char byte : output) {
    checksum += byte;
  }

  bool ok = (checksum == storedChecksum);

  // Write decompressed output
  ofstream out(outputPath, ios::binary);
  if (!out) {
    cout << "Error: cannot open " << outputPath << endl;
    return false;
  }
  out.write(output.data(), output.size());
  out.close();

  if (ok) {
    cout << "Decompressed: " << inputPath << " -> " << outputPath << endl;
    cout << "Checksum OK — file is intact." << endl;
  } else {
    cout << "Decompressed: " << inputPath << " -> " << outputPath << endl;
    cout << "WARNING: Checksum mismatch — file may be corrupted!" << endl;
  }

  return ok;
}
