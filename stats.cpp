#include <fstream>
#include <iostream>
#include <string>

using namespace std;

// Gets the size of a file in bytes
long fileSize(string path) {
  ifstream file(path, ios::binary | ios::ate);
  if (!file) {
    cout << "Error: cannot open " << path << endl;
    return -1;
  }
  return file.tellg();
}

void stats(string originalPath, string compressedPath) {
  long original = fileSize(originalPath);
  long compressed = fileSize(compressedPath);

  if (original < 0 || compressed < 0)
    return;

  double saved = 0;
  if (original > 0) {
    saved = (1.0 - (double)compressed / original) * 100.0;
  }

  cout << "Original:   " << original << " bytes" << endl;
  cout << "Compressed: " << compressed << " bytes" << endl;

  if (saved > 0) {
    cout << "Saved:      " << saved << "%" << endl;
  } else {
    cout << "Saved:      0% (file grew or stayed same)" << endl;
  }
}
