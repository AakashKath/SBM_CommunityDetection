#include "sequence_generator.h"

using namespace std;

generated_sequence generateSequence(string filename) {
    // Required Parameters
    int nodes, edges, communities, radius, algorithm_number;
    double intra_community_edge_probability, inter_community_edge_probability;
    bool uneven_node_distribution;

    string configPath = CONFIG_DIRECTORY + filename;

    // Read json file for graph configurations
    ifstream input_file(configPath);
    if (!input_file.is_open()) {
        cerr << "Could not open file " << configPath << "." << endl;
        exit(EXIT_FAILURE);
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
                                    : (algorithm_number == 3 ? "Approximate Community Detection"
                                    : (algorithm_number == 4 ? "ILP Solver"
                                    : "Unknown Algorithm"))))
                                << endl;
    cout << "For now we add edges randomly." << endl;
    if (algorithm_number == 1) {
        // Unique algorithm 1 params
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

    // Create directory for results
    string resultDirectory = to_string(nodes) + string("_") + to_string(edges) + string("_") + to_string(communities)
                            + string("_") + to_string(radius) + string("_") + to_string(int(inter_community_edge_probability * 100))
                            + string("_") + to_string(int(intra_community_edge_probability * 100));
    if (!filesystem::exists(TEST_OUTPUT_DIRECTORY + resultDirectory)) {
        filesystem::create_directories(TEST_OUTPUT_DIRECTORY + resultDirectory);
    }

    Sbm sbm(nodes, communities, intra_community_edge_probability, inter_community_edge_probability);
    sbm.sbm_graph.draw(TEST_OUTPUT_DIRECTORY + string("/original_graph.png"));

    vector<pair<int, int>> addedEdges{};
    for (int i = 0; i < edges; ++i) {
        addedEdges.push_back(sbm.generateEdge());
    }

    // TODO: Edge removal is still used for algo testing purposes
    vector<pair<int, int>> removedEdges = {};

    return generated_sequence{
        .sbm = move(sbm),
        .algorithm_number = algorithm_number,
        .radius = radius,
        .addedEdges = addedEdges,
        .removedEdges = removedEdges,
        .resultDirectory = resultDirectory
    };
}
