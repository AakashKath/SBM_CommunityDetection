#include "self_run.h"

void run_against_self() {
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    string current_datetime = ctime(&now_time);
    current_datetime.pop_back(); // Remove the newline character
    ostringstream oss;
    oss << put_time(localtime(&now_time), "%Y_%m_%d_%H_%M_%S");
    string formatted_datetime = oss.str();

    string result_directory = TEST_OUTPUT_DIRECTORY + formatted_datetime;
    if (!filesystem::exists(result_directory)) {
        filesystem::create_directories(result_directory);
    }

    vector<tuple<int, int, int, double, double>> sbm_details = {
        {1000, 10000, 5, 0.99, 0.01},
        {1000, 10000, 10, 0.99, 0.01},
        {1000, 10000, 20, 0.99, 0.01},
    };

    for (const auto& detail: sbm_details) {
        int nodes = get<0>(detail);
        int edges = get<1>(detail);
        int communities = get<2>(detail);
        double intra_community_edge_probability = get<3>(detail);
        double inter_community_edge_probability = get<4>(detail);

        string file_prefix = to_string(nodes) + string("_") + to_string(edges) + string("_") + to_string(communities)
                             + string("_") + to_string(int(inter_community_edge_probability * 100))
                             + string("_") + to_string(int(intra_community_edge_probability * 100));
        
        string path_prefix = result_directory + string("/") + file_prefix;

        Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);

        // Add edges
        vector<pair<int, int>> addedEdges{};
        for (int i = 0; i < edges; ++i) {
            addedEdges.push_back(sbm.generateEdge());
        }

        // Not using removed edges for now
        vector<pair<int, int>> removedEdges{};

        ofstream errorfile(result_directory + string("/errors.txt"));

        // Run algorithms
        try {
            ofstream accuracyfile_no_stop(path_prefix + string("_no_stop.txt"));
            ApproximateCommunityDetection acd(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges, -1, &accuracyfile_no_stop);
            plot_results(file_prefix, path_prefix, string("no_stop"));
        } catch (const exception& e) {
            errorfile << "ACD_NO_STOP: " << e.what() << endl;
        }

        try {
            ofstream accuracyfile_2_stop(path_prefix + string("_2_stop.txt"));
            ApproximateCommunityDetection acd(sbm.sbm_graph, sbm.numberCommunities, addedEdges, removedEdges, 2, &accuracyfile_2_stop);
            plot_results(file_prefix, path_prefix, string("2_stop"));
        } catch (const exception& e) {
            errorfile << "ACD_2_STOP: " << e.what() << endl;
        }
    }
}

void plot_results(string file_prefix, string path_prefix, string condition) {
    ifstream file(path_prefix + string("_") + condition+ string(".txt"));

    vector<int> x;
    vector<double> y1, y2, y3, y4;

    int edge_count;
    double node_overlap_accuracy, edge_classification_accuracy, max_jaccard_sum, maximal_matching_accuracy;
    while (file
        >> edge_count
        // >> node_overlap_accuracy
        >> edge_classification_accuracy
        >> max_jaccard_sum
        >> maximal_matching_accuracy
    ) {
        x.push_back(edge_count);
        // y1.push_back(node_overlap_accuracy);
        y2.push_back(edge_classification_accuracy);
        y3.push_back(max_jaccard_sum);
        y4.push_back(maximal_matching_accuracy);
    }
    file.close();

    // Plot the results
    // plt::named_plot("Node Overlap Accuracy", x, y1, "r-");
    plt::named_plot("Edge Classification Accuracy", x, y2, "g-");
    plt::named_plot("Max Jaccard Sum", x, y3, "b-");
    plt::named_plot("Maximal Matching Accuracy", x, y4, "y-");

    plt::ylim(0, 1);
    plt::xlabel("Number of Edges");
    plt::ylabel("Quality Measures");
    plt::title(file_prefix + string(" - ") + condition);
    plt::legend();

    // Save the plot
    plt::save(path_prefix + string("_") + condition + string(".png"));
    plt::close();
}
