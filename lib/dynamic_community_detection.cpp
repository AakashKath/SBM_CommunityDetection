#include "dynamic_community_detection.h"


DynamicCommunityDetection::DynamicCommunityDetection(const Graph& graph, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges): graph(graph), addedEdges(addedEdges), removedEdges(removedEdges) {
    c_ll = graph;
    Graph c_aux = c_ll;
    initialPartition(c_aux);
    mod = modularity(c_aux);
    old_mod = 0;
    int m = 0, n = 0;
    vector<pair<int, int>> changed_nodes;
    do {
        changed_nodes = oneLevel(c_aux);
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
        } else if (n <= removedEdges.size()) {
            auto [src, dest] = removedEdges[n - 1];
            auto [involved_communities, anodes] = affectedByRemoval(src, dest);
            removeEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            n++;
        }

        c_aux = c_ul;
    } while (mod > old_mod || m < addedEdges.size() || n < removedEdges.size());
}

DynamicCommunityDetection::~DynamicCommunityDetection() {
    // Nothing to clean
}

void DynamicCommunityDetection::initialPartition(Graph& c_aux) {
    for (Node& node: c_ll.nodes) {
        node.label = node.id;
    }
    for (Node& node: c_aux.nodes) {
        node.label = node.id;
    }
}

double DynamicCommunityDetection::modularity(Graph& auxiliary_graph) {
    int m = auxiliary_graph.getTotalEdges();

    // Modularity is 0 in case of no edges
    if (m == 0) {
        return 0.0;
    }

    double q = 0.0;
    vector<Node> nodes = auxiliary_graph.nodes;
    vector<int> degrees(nodes.size(), 0);

    // Degree counts weight on edge as well
    for (const auto& node: nodes) {
        int degreeWeight = 0;
        for (const auto& edge: node.edgeList) {
            degreeWeight += get<2>(edge);
        }
        degrees[node.id] = degreeWeight;
    }

    for (const auto& srcNode: nodes) {
        for (const auto& edge: srcNode.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            const Node& destNode = auxiliary_graph.getNode(destNodeId);
            if (srcNode.label == destNode.label) {
                q += (weight - (degrees[srcNode.id] * degrees[destNodeId]) / (2.0 * m));
            }
        }
    }

    return q / (2.0 * m);
}

vector<pair<int, int>> DynamicCommunityDetection::oneLevel(Graph& auxiliary_graph) {
    vector<pair<int, int>> changed_nodes{};
    bool moved = false;
    vector<int> nodes(c_ll.nodes.size());
    iota(nodes.begin(), nodes.end(), 0);
    random_device rd;
    mt19937 g(rd());
    shuffle(nodes.begin(), nodes.end(), g);

    for (auto& node: auxiliary_graph.nodes) {
        int current_community = node.label;
        int best_community = current_community;
        double max_modularity_gain = numeric_limits<double>::lowest();
        double initial_modularity = modularity(auxiliary_graph);

        // List neighboring communities for possible movements
        unordered_set<int> neighboring_communities;
        for (const auto& edge: node.edgeList) {
            const Node& destNode = auxiliary_graph.getNode(get<1>(edge));
            neighboring_communities.insert(destNode.label);
        }

        for (int community: neighboring_communities) {
            // Add node to the new community
            node.label = community;

            double modularity_gain = modularity(auxiliary_graph) - initial_modularity;
            if (modularity_gain > max_modularity_gain) {
                best_community = community;
                max_modularity_gain = modularity_gain;
            }
        }

        // Move node to the best community found
        node.label = best_community;
        if (best_community != current_community) {
            moved = true;
            changed_nodes.emplace_back(node.id, best_community);
        }
    }

    return changed_nodes;
}

void DynamicCommunityDetection::updateCommunities(const vector<pair<int, int>>& changed_nodes) {
    for (const auto& node_pair : changed_nodes) {
        c_ll.getNode(node_pair.first).label = node_pair.second;
    }
}

void DynamicCommunityDetection::partitionToGraph() {
    unordered_map<int, vector<int>> communities;
    for (const Node& node: c_ll.nodes) {
        communities[node.label].push_back(node.id);
    }

    Graph partitioned_graph(communities.size());

    // Update id, label, and mapping
    int index = 0;
    for (const auto& community: communities) {
        partitioned_graph.nodes[index].id = community.first;
        partitioned_graph.nodes[index].label = community.first;
        partitioned_graph.id_to_index_mapping[community.first] = index;
        index++;
    }

    // Add edges
    for (const auto& node: c_ll.nodes) {
        for (const auto& edge: node.edgeList) {
            partitioned_graph.addEdge(node.label, c_ll.getNode(get<1>(edge)).label, get<2>(edge));
        }
    }

    c_ul = partitioned_graph;
}

tuple<pair<int, int>, vector<int>> DynamicCommunityDetection::affectedByAddition(int src, int dest) {
    unordered_set<int> affected_nodes;
    affected_nodes.insert(src);
    affected_nodes.insert(dest);

    int srcCommunity = c_ll.nodes[src].label;
    int destCommunity = c_ll.nodes[dest].label;
    pair<int, int> involved_communities = {srcCommunity, destCommunity};

    for (const Node& node: c_ll.nodes) {
        if (node.label == srcCommunity || node.label == destCommunity) {
            affected_nodes.insert(node.id);
        }
    }

    return {involved_communities, vector<int>(affected_nodes.begin(), affected_nodes.end())};
}

tuple<pair<int, int>, vector<int>> DynamicCommunityDetection::affectedByRemoval(int src, int dest) {
    return affectedByAddition(src, dest);
}

void DynamicCommunityDetection::addEdge(int src, int dest) {
    c_ll.addEdge(src, dest);
}

void DynamicCommunityDetection::removeEdge(int src, int dest) {
    c_ll.removeEdge(src, dest);
}

void DynamicCommunityDetection::disbandCommunities(const vector<int>& anodes) {
    for (int nodeId : anodes) {
        c_ll.nodes[c_ll.id_to_index_mapping[nodeId]].label = nodeId;
    }
}

void DynamicCommunityDetection::syncCommunities(const pair<int, int>& involved_communities, const vector<int>& anodes) {
    // Fetch neighboring communities
    vector<int> neighboring_communities{};
    for (const auto& edge: c_ul.getNode(involved_communities.first).edgeList) {
        neighboring_communities.push_back(get<1>(edge));
    }

    // Remove pre-existing nodes
    c_ul.removeNode(involved_communities.first);
    c_ul.removeNode(involved_communities.second);

    // Add new nodes
    for (int node: anodes) {
        c_ul.addNode(node, node);
    }

    // Create edges (includes both disbanded nodes as well as pre-existing communities)
    for (int src: anodes) {
        for (const auto& edge: c_ll.getNode(src).edgeList) {
            int dest = c_ll.getNode(get<1>(edge)).label;
            c_ul.addEdge(src, dest, get<2>(edge));
        }
    }
}

unordered_map<int, int> DynamicCommunityDetection::getPredictedLabels() {
    unordered_map<int, int> predicted_labels{};
    for (const auto& node: c_ll.nodes) {
        predicted_labels[node.id] = node.label;
    }
    return predicted_labels;
}
