#include <iostream>
#include <cstring>
#include "lib/defaults.h"
#include "lib/sbm.h"
#include "lib/dynamic_community_detection.h"
#include "lib/belief_propagation.h"

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
    int radius = DEFAULT_RADIUS;
    double intra_community_edge_probability = DEFAULT_INTRA_COMMUNITY_EDGE_PROBABILITY;
    double inter_community_edge_probability = DEFAULT_INTER_COMMUNITY_EDGE_PROBABILITY;

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
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--intra_community_edge_probability") == 0) {
            if (i+1 < argc) {
                intra_community_edge_probability = stoi(argv[i + 1]);
                ++i; // skip the next argument as it's the value for probability of intra-community edge
            } else {
                cerr << "Error: --intra_community_edge_probability or -w requires a number as an argument.\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--inter_community_edge_probability") == 0) {
            if (i+1 < argc) {
                inter_community_edge_probability = stoi(argv[i + 1]);
                ++i; // skip the next argument as it's the value for probability of inter-community edge
            } else {
                cerr << "Error: --inter_community_edge_probability or -b requires a number as an argument.\n";
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

    // Make sure nodes can be equally divided into given communities
    if (nodes % communities != 0) {
        throw runtime_error("Nodes cannot be equally divided in given number of communities");
    }

    // TODO: Need to use community probability vector and edge connection matrix
    // Can also just ask for inter and intra community probabilities
    Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);
    sbm.sbm_graph.draw("output.png");

    // TODO: Should be dynamic
    vector<pair<int, int>> addedEdges = {{0, 1}, {0, 2}, {1, 2}, {2, 4}, {3, 4}, {3, 6}, {4, 5}, {5, 6}};
    vector<pair<int, int>> removedEdges = {};

    DynamicCommunityDetection dcd(sbm.sbm_graph, addedEdges, removedEdges);
    unordered_map<int, int> predicted_labels = dcd.getPredictedLabels();
    for (int i = 0; i < predicted_labels.size(); ++i) {
        cout << "Node: " << i << " Community: " << predicted_labels[i] << endl;
    }

    // BeliefPropagation bp(sbm.sbm_graph, communities, radius, intra_community_edge_probability, inter_community_edge_probability);
    // vector<int> labels = bp.getCommunityLabels();
    // for (int i = 0; i < labels.size(); ++i) {
    //     cout << "Node: " << i << " Community: " << labels[i] << endl;
    // }

    return 0;
}
