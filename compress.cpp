#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// How far back we look for matches
const int WINDOW = 4096;
// Longest match we'll encode
const int MAX_MATCH = 18;

// Finds the longest match in the sliding window.
// Returns (offset, length). Length=0 means no match found.
pair<int, int> findMatch(vector<char> &data, int pos) {
  int bestOffset = 0;
  int bestLength = 0;

  // Start of the search window
  int start = (pos - WINDOW > 0) ? pos - WINDOW : 0;

  for (int i = start; i < pos; i++) {
    int length = 0;
    // Count how many bytes match (don't read past current pos)
    while (length < MAX_MATCH && i + length < pos &&
           pos + length < (int)data.size() &&
           data[i + length] == data[pos + length]) {
      length++;
    }
    if (length > bestLength) {
      bestLength = length;
      bestOffset = pos - i;
    }
  }

  return {bestOffset, bestLength};
}

unsigned int compress(string inputPath, string outputPath) {
  // Read entire file
  ifstream in(inputPath, ios::binary);
  if (!in) {
    cout << "Error: cannot open " << inputPath << endl;
    return 0;
  }
  vector<char> data((istreambuf_iterator<char>(in)),
                    istreambuf_iterator<char>());
  in.close();

  // Compute checksum (sum of all bytes)
  unsigned int checksum = 0;
  for (unsigned char byte : data) {
    checksum += byte;
  }

  // Write compressed output
  ofstream out(outputPath, ios::binary);
  if (!out) {
    cout << "Error: cannot open " << outputPath << endl;
    return 0;
  }

  // Process tokens in groups of 8.
  // Each group starts with 1 flag byte (8 bits, one per token).
  // Bit=1 means back-reference, bit=0 means literal.
  int pos = 0;
  while (pos < (int)data.size()) {
    // Save space for the flag byte, fill it in after processing 8 tokens
    streampos flagPos = out.tellp();
    unsigned char flagByte = 0;
    out.write("\0", 1);

    for (int bit = 0; bit < 8 && pos < (int)data.size(); bit++) {
      pair<int, int> match = findMatch(data, pos);
      int offset = match.first;
      int length = match.second;

      if (length >= 3) {
        flagByte |= (1 << bit);
        unsigned short off = (unsigned short)offset;
        unsigned char len = (unsigned char)length;
        out.write((char *)&off, 2);
        out.write((char *)&len, 1);
        pos += length;
      } else {
        out.write(&data[pos], 1);
        pos++;
      }
    }

    // Go back and write the actual flag byte
    streampos endPos = out.tellp();
    out.seekp(flagPos);
    out.write((char *)&flagByte, 1);
    out.seekp(endPos);
  }

  // Write checksum at the end
  out.write((char *)&checksum, sizeof(checksum));
  out.close();

  cout << "Compressed: " << inputPath << " -> " << outputPath << endl;
  return checksum;
}
