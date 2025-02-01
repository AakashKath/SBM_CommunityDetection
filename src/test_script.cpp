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

void run_test_script(bool draw_graphs) {
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

        if (draw_graphs) {
            sbm.sbm_graph.draw(result_directory + string("/original.png"));
        }
        ofstream outfile(result_directory + string("/results.txt"));
        ofstream errorfile(result_directory + string("/errors.txt"));

        bool include_dcd = false;
        bool include_streambp = false;
        bool include_acd = false;
        bool include_ilp = false;
        DynamicCommunityDetection* dcd = nullptr;
        BeliefPropagation* bp = nullptr;
        ApproximateCommunityDetection* acd = nullptr;
        IPSolver* ip_solver = nullptr;
        chrono::duration<double, milli> dcd_duration;
        chrono::duration<double, milli> streambp_duration;
        chrono::duration<double, milli> acd_duration;
        chrono::duration<double, milli> ilp_duration;
        chrono::high_resolution_clock::time_point start_time;
        chrono::high_resolution_clock::time_point end_time;
        unordered_map<int, set<int>> community_to_node_mapping = sbm.sbm_graph.getCommunities();
        vector<set<int>> community_partition;
        // Create community partition
        for (const auto& community : community_to_node_mapping) {
            community_partition.push_back(community.second);
        }

        // Run algorithms
        try {
            start_time = chrono::high_resolution_clock::now();
            dcd = new DynamicCommunityDetection(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
            end_time = chrono::high_resolution_clock::now();
            dcd_duration = end_time - start_time;
            if (draw_graphs) {
                dcd->c_ll.draw(result_directory + string("/dcd.png"));
            }
            include_dcd = true;
        } catch (const exception& e) {
            errorfile << "DCD: " << e.what() << endl;
        }

        try {
            start_time = chrono::high_resolution_clock::now();
            bp = new BeliefPropagation(
                sbm.sbm_graph,
                sbm.numberCommunities,
                radius,
                sbm.intraCommunityEdgeProbability,
                sbm.interCommunityEdgeProbability,
                addedEdges,
                removedEdges
            );
            end_time = chrono::high_resolution_clock::now();
            streambp_duration = end_time - start_time;
            if (draw_graphs) {
                bp->bp_graph.draw(result_directory + string("/streambp.png"));
            }
            include_streambp = true;
        } catch (const exception& e) {
            errorfile << "StreamBP: " << e.what() << endl;
        }

        try {
            start_time = chrono::high_resolution_clock::now();
            acd = new ApproximateCommunityDetection(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
            end_time = chrono::high_resolution_clock::now();
            acd_duration = end_time - start_time;
            if (draw_graphs) {
                acd->acd_graph.draw(result_directory + string("/acd.png"));
            }
            include_acd = true;
        } catch (const exception& e) {
            errorfile << "ACD: " << e.what() << endl;
        }

        try {
            start_time = chrono::high_resolution_clock::now();
            ip_solver = new IPSolver(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges);
            end_time = chrono::high_resolution_clock::now();
            ilp_duration = end_time - start_time;
            if (draw_graphs) {
                ip_solver->ip_graph.draw(result_directory + string("/ilp.png"));
            }
            include_ilp = true;
        } catch (const exception& e) {
            errorfile << "ILP: " << e.what() << endl;
        }

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

        if (include_dcd) {
            outfile << "DCD Communities:" << endl;
            for (const auto& community_cluster: dcd->c_ll.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << endl;
        }
        if (include_streambp) {
            outfile << "StreamBP Communities:" << endl;
            for (const auto& community_cluster: bp->bp_graph.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << endl;
        }
        if (include_acd) {
            outfile << "ACD Communities:" << endl;
            for (const auto& community_cluster: acd->acd_graph.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << endl;
        }
        if (include_ilp) {
            outfile << "ILP Communities:" << endl;
            for (const auto& community_cluster: ip_solver->ip_graph.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << endl;
        }

        // Print runtime ranking
        unordered_map<string, double> runtime_ranking;
        if (include_dcd) {
            runtime_ranking.emplace("DCD", dcd_duration.count());
        }
        if (include_streambp) {
            runtime_ranking.emplace("StreamBP", streambp_duration.count());
        }
        if (include_acd) {
            runtime_ranking.emplace("ACD", acd_duration.count());
        }
        if (include_ilp) {
            runtime_ranking.emplace("ILP", ilp_duration.count());
        }
        int index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Runtime (in milliseconds)" << endl;
        for (const auto& rank: runtime_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print modularity ranking
        unordered_map<string, double> modularity_ranking;
        if (include_dcd) {
            modularity_ranking.emplace("DCD", modularity(dcd->c_ll));
        }
        if (include_streambp) {
            modularity_ranking.emplace("StreamBP", modularity(bp->bp_graph));
        }
        if (include_acd) {
            modularity_ranking.emplace("ACD", modularity(acd->acd_graph));
        }
        if (include_ilp) {
            modularity_ranking.emplace("ILP", modularity(ip_solver->ip_graph));
        }
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Modularity Value" << endl;
        for (const auto& rank: modularity_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print set difference accuracy ranking
        unordered_map<string, double> node_overlap_accuracy_ranking;
        if (include_dcd) {
            node_overlap_accuracy_ranking.emplace("DCD", nodeOverlapAccuracy(dcd->c_ll, community_partition, outfile, "DCD"));
        }
        if (include_streambp) {
            node_overlap_accuracy_ranking.emplace("StreamBP", nodeOverlapAccuracy(bp->bp_graph, community_partition, outfile, "StreamBP"));
        }
        if (include_acd) {
            node_overlap_accuracy_ranking.emplace("ACD", nodeOverlapAccuracy(acd->acd_graph, community_partition, outfile, "ACD"));
        }
        if (include_ilp) {
            node_overlap_accuracy_ranking.emplace("ILP", nodeOverlapAccuracy(ip_solver->ip_graph, community_partition, outfile, "ILP"));
        }
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Node Overlap Accuracy" << endl;
        for (const auto& rank: node_overlap_accuracy_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print max jaccard sum ranking
        unordered_map<string, double> jaccard_sum;
        if (include_dcd) {
            jaccard_sum.emplace("DCD", maxJaccardSum(dcd->c_ll, community_partition, outfile, "DCD"));
        }
        if (include_streambp) {
            jaccard_sum.emplace("StreamBP", maxJaccardSum(bp->bp_graph, community_partition, outfile, "StreamBP"));
        }
        if (include_acd) {
            jaccard_sum.emplace("ACD", maxJaccardSum(acd->acd_graph, community_partition, outfile, "ACD"));
        }
        if (include_ilp) {
            jaccard_sum.emplace("ILP", maxJaccardSum(ip_solver->ip_graph, community_partition, outfile, "ILP"));
        }
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Max Jaccard Sum" << endl;
        for (const auto& rank: jaccard_sum) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }

        // Print edge classification accuracy ranking
        unordered_map<string, double> edge_classification_accuracy_ranking;
        if (include_dcd) {
            edge_classification_accuracy_ranking.emplace("DCD", edgeClassificationAccuracy(dcd->c_ll, sbm.sbm_graph));
        }
        if (include_streambp) {
            edge_classification_accuracy_ranking.emplace("StreamBP", edgeClassificationAccuracy(bp->bp_graph, sbm.sbm_graph));
        }
        if (include_acd) {
            edge_classification_accuracy_ranking.emplace("ACD", edgeClassificationAccuracy(acd->acd_graph, sbm.sbm_graph));
        }
        if (include_ilp) {
            edge_classification_accuracy_ranking.emplace("ILP", edgeClassificationAccuracy(ip_solver->ip_graph, sbm.sbm_graph));
        }
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Edge Classification Accuracy" << endl;
        for (const auto& rank: edge_classification_accuracy_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;

        // Print maximal matching accuracy ranking
        unordered_map<string, double> maximal_matching_accuracy_ranking;
        if (include_dcd) {
            maximal_matching_accuracy_ranking.emplace("DCD", maximalMatchingAccuracy(dcd->c_ll, sbm.sbm_graph, outfile));
        }
        if (include_streambp) {
            maximal_matching_accuracy_ranking.emplace("StreamBP", maximalMatchingAccuracy(bp->bp_graph, sbm.sbm_graph, outfile));
        }
        if (include_acd) {
            maximal_matching_accuracy_ranking.emplace("ACD", maximalMatchingAccuracy(acd->acd_graph, sbm.sbm_graph, outfile));
        }
        if (include_ilp) {
            maximal_matching_accuracy_ranking.emplace("ILP", maximalMatchingAccuracy(ip_solver->ip_graph, sbm.sbm_graph, outfile));
        }
        index = 1;
        outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Maximal matching Accuracy" << endl;
        for (const auto& rank: maximal_matching_accuracy_ranking) {
            outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
        }
        outfile << endl;
    }
}
