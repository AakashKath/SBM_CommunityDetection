#include <iostream>
#include <cstring>
#include "lib/defaults.h"
#include "lib/sbm.h"

using namespace std;


// Function to display the help message
void displayHelp() {
    cout << "Usage: ./main [options]\n"
        << "Options:\n"
        << "  -n, --nodes [number]  Specify the number of nodes (default: " << DEFAULT_NODES << ")\n"
        << "  -c, --communities [number]  Specify the number of communities (default: " << DEFAULT_COMMUNITIES << ")\n"
        << "  -h, --help            Display this help message\n";
}

int main(int argc, char* argv[]) {
    int nodes = DEFAULT_NODES;
    int communities = DEFAULT_COMMUNITIES;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nodes") == 0) {
            if (i+1 < argc) {
                nodes = stoi(argv[i + 1]);
                ++i; // Skip the next argument as it's the value for nodes
            } else {
                cerr << "Error: --nodes or -n requires a number as an argument.\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--communities") == 0) {
            if (i+1 < argc) {
                communities = stoi(argv[i + 1]);
                ++i; // skip the next argument as it's the value for number of communities
            } else {
                cerr << "Error: --communities or -c requires a number as an argument.\n";
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

    // TODO: Need to use community probability vector and edge connection matrix
    // Can also just ask for inter and intra community probabilities
    Sbm sbm(nodes, communities);
    sbm.sbm_graph.draw("output.png");
    cout << "Graph generated" << endl;
    return 0;
}
