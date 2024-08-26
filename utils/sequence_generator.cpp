#include "sequence_generator.h"

using namespace std;

void generateSequence(const string& filename) {
    // Required Parameters
    int nodes, edges, communities, radius, algorithm_number;
    double intra_community_edge_probability, inter_community_edge_probability;
    bool uneven_node_distribution;

    // TODO: Absolute path is used for testing purposes, as relative path doesn't work with vscode debugger
    string fullPath = "/media/kath/New Volume/Study/TUM/Programmes/Thesis/SBM_CommunityDetection/" + filename;

    // Read json file for graph configurations
    ifstream input_file(fullPath);
    if (!input_file.is_open()) {
        cerr << "Could not open file " << fullPath << "." << endl;
        return;
    }

    if (filename == "default.json") {
        cout << "Using `default.json` for parameters. You can provide your own input by creating a file similar to `default.json` in root directory." << endl;
    }

    nlohmann::json jsonData;
    try {
        input_file >> jsonData;
    } catch (const nlohmann::json::parse_error& e) {
        cerr << "Parsing failed. Error: " << e.what() << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    input_file.close();

    if (jsonData.contains("nodes")) {
        nodes = jsonData["nodes"].get<int>();
    }
    if (jsonData.contains("edges")) {
        edges = jsonData["edges"].get<int>();
    }
    if (jsonData.contains("communities")) {
        communities = jsonData["communities"].get<int>();
    }
    if (jsonData.contains("radius")) {
        radius = jsonData["radius"].get<int>();
    }
    if (jsonData.contains("algorithm_number")) {
        algorithm_number = jsonData["algorithm_number"].get<int>();
    }
    if (jsonData.contains("intra_community_edge_probability")) {
        intra_community_edge_probability = jsonData["intra_community_edge_probability"].get<double>();
    }
    if (jsonData.contains("inter_community_edge_probability")) {
        inter_community_edge_probability = jsonData["inter_community_edge_probability"].get<double>();
    }
    if (jsonData.contains("uneven_node_distribution")) {
        uneven_node_distribution = jsonData["uneven_node_distribution"].get<bool>();
    }

    cout << "Using following parameters for this run:" << endl;
    cout << "Number of nodes: " << nodes << endl;
    cout << "Number of edges: " << edges << endl;
    cout << "Number of communities: " << communities << endl;
    cout << "Uneven distribution of nodes among communities: " << uneven_node_distribution << endl;
    cout << "Algorithm used: " << (algorithm_number == 1 ? "Dynamic Community Detection"
                                    : (algorithm_number == 2 ? "Belief Propagation"
                                    : "Unknown Algorithm"))
                                << endl;
    if (algorithm_number == 1) {
        cout << "For now we add edges randomly." << endl;
    }
    if (algorithm_number == 2) {
        cout << "Impact radius: " << radius << endl;
        cout << "Intra community edge probability: " << intra_community_edge_probability << endl;
        cout << "Inter community edge probability: " << inter_community_edge_probability << endl;
    }

    // Make sure nodes can be equally divided into given communities
    if (!uneven_node_distribution && nodes % communities != 0) {
        throw runtime_error("Nodes cannot be equally divided in given number of communities");
    }

    Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);
    sbm.sbm_graph.draw("original_graph.png");

    vector<pair<int, int>> addedEdges{};
    for (int i = 0; i < edges; ++i) {
        addedEdges.push_back(sbm.generateEdge());
    }

    // TODO: Edge removal is still used for algo testing purposes
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
}