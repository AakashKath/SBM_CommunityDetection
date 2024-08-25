#include <iostream>
#include <cstring>
#include "src/defaults.h"
#include "src/sbm.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"

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
    int algorithm_number = DEFAULT_ALGORITHM_NUMBER;

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
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--intra_community_edge_probability") == 0) {
            if (i+1 < argc) {
                intra_community_edge_probability = stoi(argv[i + 1]);
                ++i; // skip the next argument as it's the value for probability of intra-community edge
            } else {
                cerr << "Error: --intra_community_edge_probability or -a requires a number as an argument.\n";
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
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--algorithm") == 0) {
            if (i+1 < argc) {
                algorithm_number = stoi(argv[i + 1]);
                ++i; // skip the next argument as it's the value for algorithm to be used
            } else {
                cerr << "Error: --algorithm or -g requires a number as an argument.\n";
                cout << "Algorithm:" << endl << "\t1. Modularity Optimization" << endl << "\t2. Belief Propagation" << endl;
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

    Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);
    sbm.sbm_graph.draw("original_graph.png");

    vector<pair<int, int>> addedEdges{};
    for (int i = 0; i < 10; ++i) {
        addedEdges.push_back(sbm.generateEdge());
    }
    vector<pair<int, int>> removedEdges = {};

    if (algorithm_number == 1) {
        DynamicCommunityDetection dcd(sbm.sbm_graph, communities, addedEdges, removedEdges);
        unordered_map<int, int> predicted_labels = dcd.c_ll.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        dcd.c_ll.draw("predicted_graph.png");
    } else if (algorithm_number == 2) {
        BeliefPropagation bp(sbm.sbm_graph, communities, radius, intra_community_edge_probability, inter_community_edge_probability, addedEdges, removedEdges);
        unordered_map<int, int> predicted_labels = bp.bp_graph.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        bp.bp_graph.draw("predicted_graph.png");
    }

    return 0;
}
