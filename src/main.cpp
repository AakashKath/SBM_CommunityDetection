#include <iostream>
#include "utils/sequence_generator.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"

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

    ofstream outfile;
    // BeliefPropagation* bp;
    DynamicCommunityDetection* dcd;
    vector<tuple<string, double, long>> memory_ranking;
    size_t start_idle_time = 0;
    size_t start_total_time = 0;
    size_t end_idle_time = 0;
    size_t end_total_time = 0;
    unordered_map<int, unordered_set<int>> community_to_node_mapping;
    unordered_map<int, int> node_to_community_mapping;
    chrono::duration<double, milli> dcd_duration;

    generated_sequence gs = generateSequence(filename);
    outfile.open(TEST_OUTPUT_DIRECTORY + gs.resultDirectory + string(".txt"));

    node_to_community_mapping = gs.sbm.sbm_graph.getLabels();
    community_to_node_mapping = gs.sbm.sbm_graph.getCommunities();

    if (gs.algorithm_number == 1) {
        if (!get_cpu_times(start_idle_time, start_total_time)) {
            cerr << "Failed to get CPU times at start." << endl;
        }
        long dcd_start_ram = getRAMUsage();
        auto dcd_start = chrono::high_resolution_clock::now();
        dcd = new DynamicCommunityDetection(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
        auto dcd_end = chrono::high_resolution_clock::now();
        long dcd_end_ram = getRAMUsage();
        if (!get_cpu_times(end_idle_time, end_total_time)) {
            cerr << "Failed to get CPU times at end." << endl;
        }
        double dcd_cpu_usage = 100.0 * (1.0 - (end_idle_time - start_idle_time) / (end_total_time - start_total_time));
        memory_ranking.emplace_back("DCD", dcd_cpu_usage, dcd_end_ram - dcd_start_ram);
        dcd_duration = dcd_end - dcd_start;
        // unordered_map<int, int> predicted_labels = dcd.c_ll.getLabels();
        // for (const auto& label: predicted_labels) {
        //     cout << "Node: " << label.first << " Community: " << label.second << endl;
        // }
        // dcd.c_ll.draw("predicted_graph.png");
    } else if (gs.algorithm_number == 2) {
        if (!get_cpu_times(start_idle_time, start_total_time)) {
            cerr << "Failed to get CPU times at start." << endl;
        }
        long bp_start_ram = getRAMUsage();
        auto bp_start = chrono::high_resolution_clock::now();
        // bp = new BeliefPropagation(
        //     gs.sbm.sbm_graph,
        //     gs.sbm.numberCommunities,
        //     gs.radius,
        //     gs.sbm.intraCommunityEdgeProbability,
        //     gs.sbm.interCommunityEdgeProbability,
        //     gs.addedEdges,
        //     gs.removedEdges
        // );
        auto bp_end = chrono::high_resolution_clock::now();
        long bp_end_ram = getRAMUsage();
        if (!get_cpu_times(end_idle_time, end_total_time)) {
            cerr << "Failed to get CPU times at end." << endl;
        }
        double bp_cpu_usage = 100.0 * (1.0 - (end_idle_time - start_idle_time) / (end_total_time - start_total_time));
        memory_ranking.emplace_back("StreamBP", bp_cpu_usage, bp_end_ram - bp_start_ram);
        chrono::duration<double, milli> bp_duration = bp_end - bp_start;
        // unordered_map<int, int> predicted_labels = bp.bp_graph.getLabels();
        // for (const auto& label: predicted_labels) {
        //     cout << "Node: " << label.first << " Community: " << label.second << endl;
        // }
        // bp.bp_graph.draw("predicted_graph.png");
    }

    outfile << "Number of Nodes: " << gs.sbm.sbm_graph.nodes.size() << endl
            << "Number of edges: " << gs.addedEdges.size() << endl
            << "Number of communities: " << gs.sbm.numberCommunities << endl
            << "Radius: " << gs.radius << endl
            << "Inter-community edge probability: " << gs.sbm.interCommunityEdgeProbability << endl
            << "Intra-community edge probability: " << gs.sbm.intraCommunityEdgeProbability << endl;
    outfile << endl << "Original Communities:" << endl;
    for (const auto& community_cluster: community_to_node_mapping) {
        for (const Node& node: community_cluster.second) {
            outfile << node.id << " ";
        }
        outfile << endl;
    }
    // outfile << "StreamBP Communities:" << endl;
    // for (const auto& community_cluster: bp->bp_graph.getCommunities()) {
    //     for (const Node& node: community_cluster.second) {
    //         outfile << node.id << " ";
    //     }
    //     outfile << endl;
    // }
    outfile << "DCD Communities:" << endl;
    for (const auto& community_cluster: dcd->c_ll.getCommunities()) {
        for (const Node& node: community_cluster.second) {
            outfile << node.id << " ";
        }
        outfile << endl;
    }

    outfile << endl << "Edge details:" << endl;
    outfile << left << setw(10) << "Src" << " : " << setw(10) << "Dest" << endl;
    for (const auto& [src, dest] : gs.addedEdges) {
        outfile << left << setw(10) << src << " : " << setw(10) << dest << endl;
    }

    int index = 1;
    outfile << left << setw(6) << "Rank"
            << setw(20) << "Algorithm Name"
            << setw(30) << "CPU Usage (in milliseconds)"
            << setw(20) << "Memory Usage (in KB)" << endl;
    for (const auto& rank: memory_ranking) {
        outfile << left << setw(6) << index++
                << setw(20) << get<0>(rank)
                << setw(30) << fixed << setprecision(4) << get<1>(rank)
                << setw(20) << get<2>(rank) << endl;
    }

    unordered_map<string, double> runtime_ranking;
    runtime_ranking.emplace("DCD", dcd_duration.count());
    // runtime_ranking.emplace("StreamBP", bp_duration.count());
    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Runtime (in milliseconds)" << endl;
    for (const auto& rank: runtime_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
    outfile << endl;

    unordered_map<string, double> modularity_ranking;
    modularity_ranking.emplace("DCD", modularity(dcd->c_ll));
    // modularity_ranking.emplace("StreamBP", modularity(bp->bp_graph));
    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Modularity Value" << endl;
    for (const auto& rank: modularity_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }

    unordered_map<string, double> symmetric_difference_ranking;
    symmetric_difference_ranking.emplace("DCD", symmetricDifference(dcd->c_ll, community_to_node_mapping));
    // symmetric_difference_ranking.emplace("StreamBP", symmetricDifference(bp->bp_graph, community_to_node_mapping));
    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Symmetric Difference Value" << endl;
    for (const auto& rank: symmetric_difference_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }

    unordered_map<string, double> f1_score_ranking;
    f1_score_ranking.emplace("DCD", f1Score(dcd->c_ll, node_to_community_mapping));
    // f1_score_ranking.emplace("StreamBP", f1Score(bp->bp_graph, node_to_community_mapping));
    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "F1 Score" << endl;
    for (const auto& rank: f1_score_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }

    unordered_map<string, double> log_likelihood_ranking;
    // Using bp_graph for original log likelihood as it doesn't make changes to the edge information
    double original_ll = loglikelihood(gs.sbm.sbm_graph, dcd->c_ll);
    double dcd_ll = loglikelihood(dcd->c_ll, dcd->c_ll);
    log_likelihood_ranking.emplace("DCD", fabs((original_ll - dcd_ll) / original_ll));
    // double bp_ll = loglikelihood(bp->bp_graph, bp->bp_graph);
    // log_likelihood_ranking.emplace("StreamBP", fabs((original_ll - bp_ll) / original_ll));
    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Log-likelihood Distance" << endl;
    for (const auto& rank: log_likelihood_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }

    unordered_map<string, double> accuracy_ranking;
    accuracy_ranking.emplace("DCD", accuracy(dcd->c_ll, node_to_community_mapping, gs.sbm.numberCommunities));
    // accuracy_ranking.emplace("StreamBP", accuracy(bp->bp_graph, node_to_community_mapping, gs.sbm.numberCommunities));

    index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Accuracy" << endl;
    for (const auto& rank: accuracy_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }

    delete dcd;
    // delete bp;
    if (outfile.is_open()) {
        outfile.close();
    }
    return 0;
}
