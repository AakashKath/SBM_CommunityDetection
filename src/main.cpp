#include <iostream>
#include "utils/sequence_generator.h"

using namespace std;


// Function to display the help message
void displayHelp() {
    cout << "Usage: ./main [options]\n"
        << "Options:\n"
        << "  -f, --filename [string]  Specify the filename"
        << "  -h, --help            Display this help message\n";
}

int main(int argc, char* argv[]) {
    string filename = "default.json";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--filename") == 0) {
            if (i+1 < argc) {
                filename = argv[i + 1];
                ++i; // Skip the next argument as it's the value for nodes
            } else {
                cerr << "Error: --filename or -f requires a string as an argument.\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            displayHelp();
            return 0;
        } else {
            cerr << "Unknown option: " << argv[i] << "\n";
            displayHelp();
            return 1;
        }
    }

    generateSequence(filename);

    return 0;
}
