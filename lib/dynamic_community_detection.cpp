#include "dynamic_community_detection.h"


DynamicCommunityDetection::DynamicCommunityDetection(Graph& graph, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges): graph(graph), addedEdges(addedEdges), removedEdges(removedEdges) {
    init();
}

DynamicCommunityDetection::~DynamicCommunityDetection() {
    // Nothing to clean
}

void DynamicCommunityDetection::init() {
    c_aux = c_ll = graph;
    initialPartition();
    mod = modularity(c_aux);
    old_mod = 0;
}

void DynamicCommunityDetection::initialPartition() {
    for (Node node: c_aux.nodes) {
        node.dcdLabel = node.id;
    }
}

void DynamicCommunityDetection::run() {
    int m = 1, n = 1;
    vector<pair<int, int>> changed_nodes;
    // TODO: Change to do while
    while (mod >= old_mod || m <= addedEdges.size() || n <= removedEdges.size()) {
        changed_nodes = oneLevel();
        updateCommunities(changed_nodes);
        old_mod = mod;
        mod = modularity(c_ll);
        partitionToGraph();

        if (m <= addedEdges.size()) {
            auto [src, dest] = addedEdges[m - 1];
            auto [involved_communities, anodes] = affectedByAddition(src, dest);
            addEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            m++;
        }

        if (n <= removedEdges.size()) {
            auto [src, dest] = removedEdges[n - 1];
            auto [involved_communities, anodes] = affectedByRemoval(src, dest);
            removeEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            n++;
        }

        c_aux = c_ul;
    }
}

double DynamicCommunityDetection::modularity(Graph& auxiliary_graph) {
    int m = auxiliary_graph.getTotalEdges();
    double q = 0.0;
    vector<Node> nodes = auxiliary_graph.nodes;
    vector<int> degrees(nodes.size(), 0);

    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < nodes.size(); ++j) {
            degrees[i] += graph.adjacencyMatrix[i][j];
        }
    }

    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < nodes.size(); ++j) {
            if (nodes[i].dcdLabel == nodes[j].dcdLabel) {
                q += (graph.adjacencyMatrix[i][j] - (degrees[i] * degrees[j]) / (2.0 * m));
            }
        }
    }

    return q / (2.0 * m);
}

vector<pair<int, int>> DynamicCommunityDetection::oneLevel() {
    vector<pair<int, int>> changed_nodes{};
    bool moved = false;
    vector<int> nodes(graph.nodes.size());
    iota(nodes.begin(), nodes.end(), 0);
    random_device rd;
    mt19937 g(rd());
    shuffle(nodes.begin(), nodes.end(), g);

    for (int node : nodes) {
        int current_community = c_aux.nodes[node].dcdLabel;
        int best_community = current_community;
        double max_modularity_gain = numeric_limits<double>::lowest();
        double initial_modularity = modularity(c_aux);

        // List neighboring communities for possible movements
        unordered_set<int> neighboring_communities;
        for (size_t i = 0; i < graph.adjacencyMatrix[node].size(); ++i) {
            int neighbor = graph.adjacencyMatrix[node][i];
            if (neighbor > 0) {
                neighboring_communities.insert(c_aux.nodes[i].dcdLabel);
            }
        }

        for (int community : neighboring_communities) {
            // Add node to the new community
            c_aux.nodes[node].dcdLabel = community;

            double modularity_gain = modularity(c_aux) - initial_modularity;

            if (modularity_gain > max_modularity_gain) {
                best_community = community;
                max_modularity_gain = modularity_gain;
            }
        }

        // Move node to the best community found
        c_aux.nodes[node].dcdLabel = best_community;

        if (best_community != current_community) {
            moved = true;
            changed_nodes.emplace_back(node, best_community);
        }
    }

    return changed_nodes;
}

void DynamicCommunityDetection::updateCommunities(const vector<pair<int, int>>& changed_nodes) {
    for (const auto& node_pair : changed_nodes) {
        c_ll.nodes[node_pair.first].dcdLabel = node_pair.second;
    }
}

void DynamicCommunityDetection::partitionToGraph() {
    unordered_map<int, vector<int>> communities;
    for (const Node& node: c_ll.nodes) {
        communities[node.dcdLabel].push_back(node.id);
    }

    Graph partitioned_graph(communities.size());

    // Update id and dcdLabel
    int index = 0;
    for (const auto& community: communities) {
        partitioned_graph.nodes[index].id = community.first;
        partitioned_graph.nodes[index].dcdLabel = community.first;
        partitioned_graph.id_to_idx_mapping[community.first] = index;
        index++;
    }

    // Add edges
    for (const auto& src_community: communities) {
        for (const auto& dest_community: communities) {
            int edge_weight = 0;
            for (const auto& src_node: src_community.second) {
                for (const auto& dest_node: dest_community.second) {
                    edge_weight += c_ll.adjacencyMatrix[src_node][dest_node];
                }
            }
            int src_index = partitioned_graph.id_to_idx_mapping[src_community.first];
            int dest_index = partitioned_graph.id_to_idx_mapping[dest_community.first];
            partitioned_graph.addEdge(src_index, dest_index, edge_weight);
        }
    }

    c_ul = partitioned_graph;
}

tuple<pair<int, int>, vector<int>> DynamicCommunityDetection::affectedByAddition(int src, int dest) {
    unordered_set<int> affected_nodes;
    affected_nodes.insert(src);
    affected_nodes.insert(dest);

    int src_community = c_ll.nodes[src].dcdLabel;
    int dest_community = c_ll.nodes[dest].dcdLabel;
    pair<int, int> involved_communities = {src_community, dest_community};

    for (const Node& node: c_ll.nodes) {
        if (node.dcdLabel == src_community || node.dcdLabel == dest_community) {
            affected_nodes.insert(node.id);
        }
    }

    return {involved_communities, vector<int>(affected_nodes.begin(), affected_nodes.end())};
}

tuple<pair<int, int>, vector<int>> DynamicCommunityDetection::affectedByRemoval(int src, int dest) {
    return affectedByAddition(src, dest);
}

void DynamicCommunityDetection::addEdge(int src, int dest) {
    graph.addEdge(src, dest);
    c_ll.addEdge(src, dest);
}

void DynamicCommunityDetection::removeEdge(int src, int dest) {
    graph.removeEdge(src, dest);
    graph.removeEdge(src, dest);
}

void DynamicCommunityDetection::disbandCommunities(const vector<int>& anodes) {
    for (int node : anodes) {
        c_ll.nodes[node].dcdLabel = node;
    }
}

void DynamicCommunityDetection::syncCommunities(const pair<int, int>& involved_communities, const vector<int>& anodes) {
    int src_idx = c_ul.id_to_idx_mapping[involved_communities.first];
    int dest_idx = c_ul.id_to_idx_mapping[involved_communities.second];

    vector<int> neighboring_communities{};
    // Fetch neighboring communities
    for (int i = 0; i < c_ul.nodes.size(); ++i) {
        if ((src_idx != i && c_ul.adjacencyMatrix[src_idx][i] > 0) || (dest_idx != i && c_ul.adjacencyMatrix[dest_idx][i] > 0)) {
            neighboring_communities.push_back(c_ul.getIdFromIdx(i));
        }
    }

    // Remove pre-existing nodes
    c_ul.removeNode(src_idx);
    c_ul.removeNode(dest_idx);

    // Add new nodes
    for (int node: anodes) {
        c_ul.addNode(node, node);
    }

    // Create edges between disbanded nodes
    for (int src: anodes) {
        for (int dest: anodes) {
            if (c_ll.adjacencyMatrix[src][dest] > 0) {
                c_ul.addEdge(c_ul.id_to_idx_mapping[src], c_ul.id_to_idx_mapping[dest]);
            }
        }
    }

    // Add edges from disbanded nodes to pre-existing communities
    for (int i = 0; i < c_ll.nodes.size(); ++i) {
        for (int node: anodes) {
            c_ul.addEdge(c_ul.id_to_idx_mapping[node], c_ul.id_to_idx_mapping[c_ll.nodes[i].dcdLabel], c_ll.adjacencyMatrix[node][i]);
        }
    }
}

unordered_map<int, int> DynamicCommunityDetection::getPredictedLabels() {
    unordered_map<int, int> predicted_labels{};
    for (const auto& node: c_ll.nodes) {
        predicted_labels[node.id] = node.dcdLabel;
    }
    return predicted_labels;
}
