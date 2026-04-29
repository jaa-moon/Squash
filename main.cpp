#include <iostream>
#include <string>

// Defined in compress.cpp, decompress.cpp, stats.cpp
unsigned int compress(std::string inputPath, std::string outputPath);
bool decompress(std::string inputPath, std::string outputPath);
void stats(std::string originalPath, std::string compressedPath);

using namespace std;

void help() {
  cout << "Usage:" << endl;
  cout << "  ./squash compress   <input> <output>    Compress a file" << endl;
  cout << "  ./squash decompress <input> <output>    Decompress a .lz file"
       << endl;
  cout << "  ./squash stats      <original> <compressed>  Show size comparison"
       << endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    help();
    return 1;
  }

  string command = argv[1];

  if (command == "compress" && argc == 4) {
    compress(argv[2], argv[3]);
  } else if (command == "decompress" && argc == 4) {
    decompress(argv[2], argv[3]);
  } else if (command == "stats" && argc == 4) {
    stats(argv[2], argv[3]);
  } else {
    help();
    return 1;
  }

  return 0;
}
