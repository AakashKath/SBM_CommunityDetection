#include "dynamic_community_detection.h"


DynamicCommunityDetection::DynamicCommunityDetection(Graph graph, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges): c_ll(graph), c_ul(Graph(0)) {
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

        if (m < addedEdges.size()) {
            auto [src, dest] = addedEdges[m];
            auto [involved_communities, anodes] = affectedByAddition(src, dest);
            c_ll.addUndirectedEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            m++;
        } else if (n < removedEdges.size()) {
            auto [src, dest] = removedEdges[n];
            auto [involved_communities, anodes] = affectedByRemoval(src, dest);
            c_ll.removeUndirectedEdge(src, dest);
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

double DynamicCommunityDetection::modularity(const Graph& auxiliary_graph) const {
    int m = auxiliary_graph.getTotalEdges();

    // Modularity is 0 in case of no edges
    if (m == 0) {
        return 0.0;
    }

    double q = 0.0;
    unordered_map<int, int> degrees{};

    // Degree counts weight on edge as well
    for (const Node& node: auxiliary_graph.nodes) {
        int degreeWeight = 0;
        for (const auto& edge: node.edgeList) {
            degreeWeight += get<2>(edge);
        }
        degrees.emplace(node.id, degreeWeight);
    }

    for (const Node& srcNode: auxiliary_graph.nodes) {
        for (const auto& edge: srcNode.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            const Node& destNode = auxiliary_graph.getNode(destNodeId);
            try {
                if (srcNode.label == destNode.label) {
                    q += (weight - static_cast<double>(degrees.at(srcNode.id) * degrees.at(destNodeId)) / (2.0 * m));
                }
            } catch (const out_of_range& e) {
                throw out_of_range("Node " + to_string(srcNode.id) + " or " + to_string(destNodeId) + " not found.");
            }
        }
    }

    return q / (2.0 * m);
}

vector<pair<int, int>> DynamicCommunityDetection::oneLevel(Graph& auxiliary_graph) {
    unordered_map<int, vector<int>> communities;
    for (const Node& node: auxiliary_graph.nodes) {
        communities[node.label].push_back(node.id);
    }

    unordered_set<int> changed_aux_nodes{};
    vector<int> node_indices(auxiliary_graph.nodes.size());
    iota(node_indices.begin(), node_indices.end(), 0);
    random_device rd;
    mt19937 g(rd());
    shuffle(node_indices.begin(), node_indices.end(), g);
    double initial_modularity = modularity(auxiliary_graph);

    for (int idx: node_indices) {
        Node& node = auxiliary_graph.nodes[idx];

        int current_community = node.label;
        int best_community = current_community;
        double max_modularity_gain = numeric_limits<double>::lowest();
        double best_modularity;

        // List neighboring communities for possible movements
        unordered_set<int> neighboring_communities;
        for (const auto& edge: node.edgeList) {
            const Node& destNode = auxiliary_graph.getNode(get<1>(edge));
            // Skip if same as current community
            if (destNode.label != current_community) {
                neighboring_communities.insert(destNode.label);
            }
        }

        for (int community: neighboring_communities) {
            // Temporarily move node to new community
            try {
                for (int nodeId: communities.at(current_community)) {
                    Node& movedNode = auxiliary_graph.getNode(nodeId);
                    movedNode.label = community;
                }
            } catch (const out_of_range& e) {
                cerr << "Cannot move to temp community. Community " << current_community << " not found. Please check the erase code at the end of the method." << endl;
            }

            double updated_modularity = modularity(auxiliary_graph);

            double modularity_gain = updated_modularity - initial_modularity;
            if (modularity_gain >= 0 && modularity_gain > max_modularity_gain) {
                best_community = community;
                best_modularity = updated_modularity;
                max_modularity_gain = modularity_gain;
            }

            // Move the node back to its original community
            try {
                for (int nodeId: communities.at(current_community)) {
                    Node& movedNode = auxiliary_graph.getNode(nodeId);
                    movedNode.label = current_community;
                }
            } catch (const out_of_range& e) {
                cerr << "Cannot revert back to original community. Community " << current_community << " not found. Please check the erase code at the end of the method." << endl;
            }
        }

        // Move node to the best community found
        if (best_community != current_community) {
            initial_modularity = best_modularity;
            try {
                for (int nodeId: communities.at(current_community)) {
                    Node& movedNode = auxiliary_graph.getNode(nodeId);
                    movedNode.label = best_community;
                    changed_aux_nodes.insert(nodeId);
                }
            } catch (const out_of_range& e) {
                cerr << "Cannot move to best community. Community " << current_community << " not found. Please check the erase code at the end of the method." << endl;
            }
            try {
                communities.at(best_community).insert(communities.at(best_community).end(), communities.at(current_community).begin(), communities.at(current_community).end());
                communities.erase(current_community);
            } catch (const out_of_range& e) {
                cerr << "Cannot delete communities. Community " << current_community << " or " << best_community << " not found. Please check the erase code at the end of the method." << endl;
            } catch (const exception& e) {
                throw runtime_error("An unexpected error occurred: " + string(e.what()));
            }
        }
    }

    // Get changed nodes from auxiliary graph
    vector<pair<int, int>> changed_nodes{};
    for (const Node& node: c_ll.nodes) {
        if (changed_aux_nodes.find(node.label) != changed_aux_nodes.end()) {
            const Node& aux_node = auxiliary_graph.getNode(node.label);
            changed_nodes.emplace_back(node.id, aux_node.label);
        }
    }

    return changed_nodes;
}

void DynamicCommunityDetection::updateCommunities(const vector<pair<int, int>>& changed_nodes) {
    for (const auto& node_pair : changed_nodes) {
        Node& node = c_ll.getNode(node_pair.first);
        node.label = node_pair.second;
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
    unordered_map<int, size_t> new_id_to_index_mapping;
    for (const auto& community: communities) {
        partitioned_graph.nodes[index].id = community.first;
        partitioned_graph.nodes[index].label = community.first;
        new_id_to_index_mapping.emplace(community.first, index);
        index++;
    }
    partitioned_graph.id_to_index_mapping = new_id_to_index_mapping;

    // Add edges
    for (const auto& node: c_ll.nodes) {
        int srcCommunity = node.label;
        for (const auto& edge: node.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            int destCommunity = c_ll.getNode(destNodeId).label;
            partitioned_graph.addEdge(srcCommunity, destCommunity, weight);
        }
    }

    c_ul = partitioned_graph;
}

pair<pair<int, int>, unordered_set<int>> DynamicCommunityDetection::affectedByAddition(int src, int dest) const {
    unordered_set<int> affected_nodes;
    affected_nodes.insert(src);
    affected_nodes.insert(dest);

    int srcCommunity = c_ll.getNode(src).label;
    int destCommunity = c_ll.getNode(dest).label;
    pair<int, int> involved_communities = {srcCommunity, destCommunity};

    for (const Node& node: c_ll.nodes) {
        if (node.label == srcCommunity || node.label == destCommunity) {
            affected_nodes.insert(node.id);
        }
    }

    return {involved_communities, affected_nodes};
}

pair<pair<int, int>, unordered_set<int>> DynamicCommunityDetection::affectedByRemoval(int src, int dest) const {
    return affectedByAddition(src, dest);
}

void DynamicCommunityDetection::disbandCommunities(const unordered_set<int>& anodes) {
    for (int nodeId : anodes) {
        Node& node = c_ll.getNode(nodeId);
        node.label = nodeId;
    }
}

void DynamicCommunityDetection::syncCommunities(const pair<int, int>& involved_communities, const unordered_set<int>& anodes) {
    // Remove pre-existing nodes
    c_ul.removeNode(involved_communities.first);
    if (involved_communities.first != involved_communities.second) {
        c_ul.removeNode(involved_communities.second);
    }

    // Add new nodes
    for (int node: anodes) {
        c_ul.addNode(node, node);
    }

    // Create edges (includes both disbanded nodes as well as pre-existing communities)
    for (int src: anodes) {
        const Node& srcNode = c_ll.getNode(src);
        for (const auto& edge: srcNode.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            const Node& destNode = c_ll.getNode(destNodeId);

            if (anodes.find(destNodeId) != anodes.end()) {
                c_ul.addEdge(src, destNode.label, weight);
            } else {
                c_ul.addUndirectedEdge(src, destNode.label, weight);
            }
        }
    }
}
