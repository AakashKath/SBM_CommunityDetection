#include "test_script.h"

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void run_test_script() {
    // Run the test script
    if (!filesystem::exists(TEST_DATA_DIRECTORY) || !filesystem::is_directory(TEST_DATA_DIRECTORY)) {
        cerr << "Test data directory not found. Please check the path." << endl;
    }

    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    string current_datetime = ctime(&now_time);
    current_datetime.pop_back(); // Remove the newline character
    ostringstream oss;
    oss << put_time(localtime(&now_time), "%Y_%m_%d_%H_%M_%S");
    string formatted_datetime = oss.str();

    for (const auto& subdirectory: filesystem::directory_iterator(TEST_DATA_DIRECTORY)) {
        string result_directory = TEST_OUTPUT_DIRECTORY + formatted_datetime + "/" + string(subdirectory.path().filename());
        if (!filesystem::exists(result_directory)) {
            filesystem::create_directories(result_directory);
        }

        // Get subdirectory name
        vector<string> tokens = splitString(subdirectory.path().filename(), '_');

        int nodes = stoi(tokens[0]);
        int edges = stoi(tokens[1]);
        int communities = stoi(tokens[2]);
        int radius = stoi(tokens[3]);
        double inter_community_edge_probability = stod(tokens[4])/100;
        double intra_community_edge_probability = stod(tokens[5])/100;

        Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);

        // Read labels file
        filesystem::path labels_file = subdirectory.path() / "labels.txt";
        ifstream labels_stream(labels_file);
        int node_id, label, offset;
        while (labels_stream >> node_id >> label >> offset) {
            Node* node = sbm.sbm_graph.getNode(node_id);
            node->label = label;
            node->offset = offset;
        }

        // Read edges file
        filesystem::path edges_file = subdirectory.path() / "edges.txt";
        ifstream edges_stream(edges_file);
        int node1_id, node2_id;
        vector<pair<int, int>> addedEdges{};
        while (edges_stream >> node1_id >> node2_id) {
            addedEdges.push_back({node1_id, node2_id});
        }

        // Not using removed edges for now
        vector<pair<int, int>> removedEdges{};

        sbm.sbm_graph.draw(result_directory + string("/original.png"));

        chrono::high_resolution_clock::time_point start_time;
        chrono::high_resolution_clock::time_point end_time;
        unordered_map<int, set<int>> community_to_node_mapping = sbm.sbm_graph.getCommunities();
        vector<set<int>> community_partition;
        // Create community partition
        for (const auto& community : community_to_node_mapping) {
            community_partition.push_back(community.second);
        }

        // Run algorithms
        start_time = chrono::high_resolution_clock::now();
        DynamicCommunityDetection dcd(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
        end_time = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> dcd_duration = end_time - start_time;
        dcd.c_ll.draw(result_directory + string("/dcd.png"));

        start_time = chrono::high_resolution_clock::now();
        BeliefPropagation bp(
            sbm.sbm_graph,
            sbm.numberCommunities,
            radius,
            sbm.intraCommunityEdgeProbability,
            sbm.interCommunityEdgeProbability,
            addedEdges,
            removedEdges
        );
        end_time = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> streambp_duration = end_time - start_time;
        bp.bp_graph.draw(result_directory + string("/streambp.png"));

        start_time = chrono::high_resolution_clock::now();
        ApproximateCommunityDetection acd(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
        end_time = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> acd_duration = end_time - start_time;
        acd.acd_graph.draw(result_directory + string("/acd.png"));

        start_time = chrono::high_resolution_clock::now();
        IPSolver ip_solver(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
        end_time = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> ilp_duration = end_time - start_time;
        ip_solver.ip_graph.draw(result_directory + string("/ilp.png"));

        // Gather results
        ofstream outfile(result_directory + string("/results.txt"));

        // Print graph details
        outfile << "Number of Nodes: " << sbm.sbm_graph.nodes.size() << endl
                << "Number of edges: " << addedEdges.size() << endl
                << "Number of communities: " << sbm.numberCommunities << endl
                << "Radius: " << radius << endl
                << "Inter-community edge probability: " << sbm.interCommunityEdgeProbability << endl
                << "Intra-community edge probability: " << sbm.intraCommunityEdgeProbability << endl;

        // Print original and expected communities
        outfile << endl << "Original Communities:" << endl;
        for (const auto& community_cluster: community_to_node_mapping) {
            for (const Node& node: community_cluster.second) {
                outfile << node.id << " ";
            }
            outfile << endl;
        }
        outfile << endl;
        outfile << "DCD Communities:" << endl;
        for (const auto& community_cluster: dcd.c_ll.getCommunities()) {
            for (const Node& node: community_cluster.second) {
                outfile << node.id << " ";
            }
            outfile << endl;
        }
        outfile << endl;
        outfile << "StreamBP Communities:" << endl;
        for (const auto& community_cluster: bp.bp_graph.getCommunities()) {
            for (const Node& node: community_cluster.second) {
                outfile << node.id << " ";
            }
            outfile << endl;
        }
        outfile << endl;
        outfile << "ACD Communities:" << endl;
        for (const auto& community_cluster: acd.acd_graph.getCommunities()) {
            for (const Node& node: community_cluster.second) {
                outfile << node.id << " ";
            }
            outfile << endl;
        }
        outfile << endl;
        outfile << "ILP Communities:" << endl;
        for (const auto& community_cluster: ip_solver.ip_graph.getCommunities()) {
            for (const Node& node: community_cluster.second) {
                outfile << node.id << " ";
            }
            outfile << endl;
        }
        outfile << endl;

        // Print runtime ranking
        unordered_map<string, double> runtime_ranking;
        runtime_ranking.emplace("DCD", dcd_duration.count());
        runtime_ranking.emplace("StreamBP", streambp_duration.count());
        runtime_ranking.emplace("ACD", acd_duration.count());
        runtime_ranking.emplace("ILP", ilp_duration.count());
        int index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Runtime (in milliseconds)" << endl;
        for (const auto& rank: runtime_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print modularity ranking
        unordered_map<string, double> modularity_ranking;
        modularity_ranking.emplace("DCD", modularity(dcd.c_ll));
        modularity_ranking.emplace("StreamBP", modularity(bp.bp_graph));
        modularity_ranking.emplace("ACD", modularity(acd.acd_graph));
        modularity_ranking.emplace("ILP", modularity(ip_solver.ip_graph));
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Modularity Value" << endl;
        for (const auto& rank: modularity_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print accuracy ranking
        unordered_map<string, double> accuracy_ranking;
        accuracy_ranking.emplace("DCD", accuracy(dcd.c_ll, community_partition, outfile, "DCD"));
        accuracy_ranking.emplace("StreamBP", accuracy(bp.bp_graph, community_partition, outfile, "StreamBP"));
        accuracy_ranking.emplace("ACD", accuracy(acd.acd_graph, community_partition, outfile, "ACD"));
        accuracy_ranking.emplace("ILP", accuracy(ip_solver.ip_graph, community_partition, outfile, "ILP"));
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Accuracy" << endl;
        for (const auto& rank: accuracy_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print max jaccard sum ranking
        unordered_map<string, double> jaccard_sum;
        jaccard_sum.emplace("DCD", maxJaccardSum(dcd.c_ll, community_partition, outfile, "DCD"));
        jaccard_sum.emplace("StreamBP", maxJaccardSum(bp.bp_graph, community_partition, outfile, "StreamBP"));
        jaccard_sum.emplace("ACD", maxJaccardSum(acd.acd_graph, community_partition, outfile, "ACD"));
        jaccard_sum.emplace("ILP", maxJaccardSum(ip_solver.ip_graph, community_partition, outfile, "ILP"));
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Max Jaccard Sum" << endl;
        for (const auto& rank: jaccard_sum) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
    }
}
