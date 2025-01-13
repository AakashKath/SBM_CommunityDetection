#include <iostream>
#include "utils/sequence_generator.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include "approximate_community_detection.h"
#include "ip_solver.h"
#include "test_script.h"

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
    bool test_script = false;
    bool draw_graphs = false;

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
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test_script") == 0) {
            test_script = true;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--draw_graphs") == 0) {
            draw_graphs = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            displayHelp();
            return 0;
        } else {
            cerr << "Unknown option: " << argv[i] << "\n";
            displayHelp();
            return 1;
        }
    }

    if (test_script) {
        // Run the test script
        run_test_script(draw_graphs);
        return 0;
    }

    generated_sequence gs = generateSequence(filename);

    if (gs.algorithm_number == 1) {
        DynamicCommunityDetection dcd(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
        unordered_map<int, int> predicted_labels = dcd.c_ll.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        dcd.c_ll.draw(TEST_OUTPUT_DIRECTORY + string("/predicted_graph.png"));
    } else if (gs.algorithm_number == 2) {
        BeliefPropagation bp(
            gs.sbm.sbm_graph,
            gs.sbm.numberCommunities,
            gs.radius,
            gs.sbm.intraCommunityEdgeProbability,
            gs.sbm.interCommunityEdgeProbability,
            gs.addedEdges,
            gs.removedEdges
        );
        unordered_map<int, int> predicted_labels = bp.bp_graph.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        bp.bp_graph.draw(TEST_OUTPUT_DIRECTORY + string("/predicted_graph.png"));
    } else if (gs.algorithm_number == 3) {
        ApproximateCommunityDetection acd(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
        unordered_map<int, int> predicted_labels = acd.acd_graph.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        acd.acd_graph.draw(TEST_OUTPUT_DIRECTORY + string("/predicted_graph.png"));
    } else if (gs.algorithm_number == 4) {
        IPSolver ip_solver(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
        unordered_map<int, int> predicted_labels = ip_solver.ip_graph.getLabels();
        for (const auto& label: predicted_labels) {
            cout << "Node: " << label.first << " Community: " << label.second << endl;
        }
        ip_solver.ip_graph.draw(TEST_OUTPUT_DIRECTORY + string("/predicted_graph.png"));
    }

    return 0;
}
