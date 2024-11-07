#include "dynamic_community_detection.h"


DynamicCommunityDetection::DynamicCommunityDetection(
    Graph graph,
    int communityCount,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    c_ll(graph),
    c_ul(Graph(0)),
    communityCount(communityCount),
    totalEdges(graph.getTotalEdges())
{
    // Assign each node to its individual community
    for (auto& node: c_ll.nodes) {
        node->label = node->id;
    }

    initialPartition(c_ll);
    Graph c_aux = c_ll;
    double mod = modularity(c_aux, totalEdges);
    double old_mod = 0.0;
    int m = 0, n = 0;
    do {
        vector<pair<int, int>> changed_nodes = oneLevel(c_aux);
        updateCommunities(changed_nodes);
        old_mod = mod;
        mod = modularity(c_ll, totalEdges);
        partitionToGraph();

        if (m < addedEdges.size()) {
            auto [src, dest] = addedEdges[m];
            auto [involved_communities, anodes] = affectedByAddition(src, dest);
            c_ll.addUndirectedEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            totalEdges++;
            m++;
        } else if (n < removedEdges.size()) {
            auto [src, dest] = removedEdges[n];
            auto [involved_communities, anodes] = affectedByRemoval(src, dest);
            c_ll.removeUndirectedEdge(src, dest);
            disbandCommunities(anodes);
            syncCommunities(involved_communities, anodes);
            // TODO: Need to subtract edgeWeight from totalEdges
            totalEdges--;
            n++;
        }

        c_aux = c_ul;
    } while (mod > old_mod || m < addedEdges.size() || n < removedEdges.size());

    // Merge communities to expected number (Using Best Fit bin packing algorithm)
    // mergeCommunities(); // TODO: Disable merge algorithm
    relabelGraph();
}

DynamicCommunityDetection::~DynamicCommunityDetection() {
    // Nothing to clean
}

void DynamicCommunityDetection::initialPartition(Graph& auxiliary_graph) {
    vector<Node*> node_list;
    for (const auto& node_ptr : auxiliary_graph.nodes) {
        node_list.push_back(node_ptr.get());
    }
    mt19937 g(rd());

    double initial_mod;
    double new_mod = modularity(auxiliary_graph, totalEdges);
    do {
        shuffle(node_list.begin(), node_list.end(), g);

        for (auto& node: node_list) {
            int current_community = node->label;
            int best_community = current_community;
            double max_mod_gain = 0.0;

            // Fetch neighboring communities
            unordered_set<int> neighboring_communities;
            for (const auto& edge: node->edgeList) {
                const Node* neighbor = edge.first;
                // Skip current community
                if (neighbor->label != current_community) {
                    neighboring_communities.insert(neighbor->label);
                }
            }

            // Find the community with highest modularity change
            for (int community: neighboring_communities) {
                // Temp move to neighboring community
                node->label = community;

                // Check modularity gain
                double mod_gain = modularity_gain(node, community, current_community);

                // Note down the best community with highest modularity change
                if (mod_gain > 0 && mod_gain > max_mod_gain) {
                    max_mod_gain = mod_gain;
                    best_community = community;
                }

                // Revert back to original community
                node->label = current_community;
            }

            // Move node to its best community
            if (current_community != best_community) {
                node->label = best_community;
            }
        }
        initial_mod = new_mod;
        new_mod = modularity(auxiliary_graph, totalEdges);
    } while (new_mod > initial_mod);
}

vector<pair<int, int>> DynamicCommunityDetection::oneLevel(Graph& auxiliary_graph) {
    vector<Node*> node_list;
    for (const auto& node_ptr : auxiliary_graph.nodes) {
        node_list.push_back(node_ptr.get());
    }
    mt19937 g(rd());
    shuffle(node_list.begin(), node_list.end(), g);
    vector<pair<int, int>> changed_nodes{};

    for (auto& node: node_list) {
        int current_community = node->label;
        int best_community = current_community;
        double max_mod_gain = 0.0;

        // Fetch neighboring communities
        unordered_set<int> neighboring_communities;
        for (const auto& edge: node->edgeList) {
            const Node* neighbor = edge.first;
            // Skip current community
            if (neighbor->label != current_community) {
                neighboring_communities.insert(neighbor->label);
            }
        }

        // Find the community with highest modularity change
        for (int community: neighboring_communities) {
            // Temp move to neighboring community
            node->label = community;

            // Check modularity gain
            double mod_gain = modularity_gain(node, community, current_community);

            // Note down the best community with highest modularity change
            if (mod_gain > max(epsilon_gain, max_mod_gain)) {
                max_mod_gain = mod_gain;
                best_community = community;
            }

            // Revert back to original community
            node->label = current_community;
        }

        // Move node to its best community
        if (current_community != best_community) {
            node->label = best_community;
            changed_nodes.emplace_back(node->id, best_community);
        }
    }

    return changed_nodes;
}

void DynamicCommunityDetection::updateCommunities(const vector<pair<int, int>>& changed_nodes) {
    // List nodes as per communities
    unordered_map<int, vector<Node*>> communities;
    for (const auto& node: c_ll.nodes) {
        communities[node->label].push_back(node.get());
    }

    // Move all grouped nodes to the new community
    for (const auto& node_pair : changed_nodes) {
        for (Node* node: communities.at(node_pair.first)) {
            node->label = node_pair.second;
        }
    }
}

void DynamicCommunityDetection::partitionToGraph() {
    unordered_map<int, vector<int>> communities;
    for (const auto& node: c_ll.nodes) {
        communities[node->label].push_back(node->id);
    }

    Graph partitioned_graph(communities.size());

    // Update id, label, and mapping
    int index = 0;
    unordered_map<int, size_t> new_id_to_index_mapping;
    for (const auto& community: communities) {
        partitioned_graph.nodes[index]->id = community.first;
        partitioned_graph.nodes[index]->label = community.first;
        new_id_to_index_mapping.emplace(community.first, index);
        index++;
    }
    partitioned_graph.id_to_index_mapping = new_id_to_index_mapping;

    // Add edges
    for (const auto& node: c_ll.nodes) {
        Node* srcCommunity = partitioned_graph.getNode(node->label);
        for (const auto& edge: node->edgeList) {
            int weight = edge.second;
            Node* destCommunity = partitioned_graph.getNode(edge.first->label);
            srcCommunity->addEdge(destCommunity, weight);
        }
    }

    c_ul = move(partitioned_graph);
}

pair<pair<Node*, Node*>, unordered_set<Node*>> DynamicCommunityDetection::affectedByAddition(int src, int dest) {
    unordered_set<Node*> affected_nodes;
    Node* srcNode = c_ll.getNode(src);
    Node* destNode = c_ll.getNode(dest);
    affected_nodes.insert(srcNode);
    affected_nodes.insert(destNode);

    Node* srcCommunity = c_ll.getNode(srcNode->label);
    Node* destCommunity = c_ll.getNode(destNode->label);
    pair<Node*, Node*> involved_communities = {srcCommunity, destCommunity};

    for (const auto& node: c_ll.nodes) {
        if (node->label == srcNode->label || node->label == destNode->label) {
            affected_nodes.insert(node.get());
        }
    }

    return {involved_communities, affected_nodes};
}

pair<pair<Node*, Node*>, unordered_set<Node*>> DynamicCommunityDetection::affectedByRemoval(int src, int dest) {
    return affectedByAddition(src, dest);
}

void DynamicCommunityDetection::disbandCommunities(unordered_set<Node*>& anodes) {
    for (auto& node: anodes) {
        node->label = node->id;
    }
}

void DynamicCommunityDetection::syncCommunities(pair<Node*, Node*>& involved_communities, unordered_set<Node*>& anodes) {
    // Remove pre-existing nodes
    c_ul.removeNode(involved_communities.first->id);
    if (involved_communities.first != involved_communities.second) {
        c_ul.removeNode(involved_communities.second->id);
    }

    // Add new nodes
    for (Node* node: anodes) {
        c_ul.addNode(node->id, node->id);
    }

    // Create edges (includes both disbanded nodes as well as pre-existing communities)
    for (Node* src: anodes) {
        for (const auto& edge: src->edgeList) {
            Node* dest = edge.first;
            int weight = edge.second;

            if (anodes.find(dest) != anodes.end()) {
                Node* c_ul_srcNode = c_ul.getNode(src->id);
                Node* c_ul_destNode = c_ul.getNode(dest->label);
                c_ul_srcNode->addEdge(c_ul_destNode, weight);
            } else {
                c_ul.addUndirectedEdge(src->id, dest->label, weight);
            }
        }
    }
}

// Not used anymore
// void DynamicCommunityDetection::mergeCommunities() {
//     unordered_map<int, vector<int>> communityMap{};
//     for (const Node& node: c_ll.nodes) {
//         communityMap[node.label].push_back(node.id);
//     }
//     vector<pair<int, vector<int>>> communityVector(communityMap.begin(), communityMap.end());
//     sort(communityVector.begin(), communityVector.end(), [](const pair<int, vector<int>>& left, const pair<int, vector<int>>& right) {
//         return left.second.size() > right.second.size();
//     });

//     int binCapacity = ceil(c_ll.nodes.size() / communityCount);
//     vector<tuple<int, int, vector<int>>> binStore;

//     int min, minIndex, binRemainingCapacity;
//     for (auto community: communityVector) {
//         int communitySize = community.second.size();

//         // Add new separate entry if community size is already more than expected
//         if (communitySize >= binCapacity) {
//             binStore.emplace_back(0, community.first, community.second);
//             continue;
//         }

//         min = binCapacity + 1;
//         for (int i = 0; i < binStore.size(); ++i) {
//             binRemainingCapacity = get<0>(binStore[i]);
//             if (binRemainingCapacity >= communitySize && binRemainingCapacity - communitySize < min) {
//                 min = binRemainingCapacity - communitySize;
//                 minIndex = i;
//             }
//         }

//         if (min == binCapacity + 1) {
//             binStore.emplace_back(binCapacity - communitySize, community.first, community.second);
//         } else {
//             get<0>(binStore[minIndex]) -= communitySize;
//             get<2>(binStore[minIndex]).insert(get<2>(binStore[minIndex]).end(), community.second.begin(), community.second.end());
//         }
//     }

//     // Update the merged community labels
//     for (auto bin: binStore) {
//         int updatedLabel = get<1>(bin);
//         for (int nodeId: get<2>(bin)) {
//             Node& node = c_ll.getNode(nodeId);
//             node.label = updatedLabel;
//         }
//     }
// }

double DynamicCommunityDetection::modularity_gain(const Node* node, int new_community, int old_community) {
    double mod_gain = 0.0;

    for (const auto& edge: node->edgeList) {
        const Node* neighbor = edge.first;
        int weight = edge.second;
        if (neighbor->label == new_community) {
            mod_gain += (static_cast<double>(weight) - (static_cast<double>(node->edgeList.size() * neighbor->edgeList.size()) / (2.0 * totalEdges))) / (2.0 * totalEdges);
        } else if (neighbor->label == old_community) {
            mod_gain -= (static_cast<double>(weight) - (static_cast<double>(node->edgeList.size() * neighbor->edgeList.size()) / (2.0 * totalEdges))) / (2.0 * totalEdges);
        }
    }

    return mod_gain;
}

void DynamicCommunityDetection::relabelGraph() {
    unordered_map<int, int> updated_label_map;
    int index = 0;
    for (auto& node: c_ll.nodes) {
        try {
            node->label = updated_label_map.at(node->label);
        } catch (const out_of_range& e) {
            updated_label_map[node->label] = index;
            node->label = index;
            index++;
        }
    }
}
