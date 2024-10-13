#include "dynamic_community_detection.h"


DynamicCommunityDetection::DynamicCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges): c_ll(graph), c_ul(Graph(0)), communityCount(communityCount) {
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
            c_ll.addUndirectedEdge(src, dest, 1, true);
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

    // Merge communities to expected number (Using Best Fit bin packing algorithm)
    // mergeCommunities(); // TODO: Disable merge algorithm
}

DynamicCommunityDetection::~DynamicCommunityDetection() {
    // Nothing to clean
}

void DynamicCommunityDetection::initialPartition(Graph& c_aux) {
    // Initialize noise as random numbers
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, communityCount - 2);
    uniform_real_distribution<double> prob_dist(0.0, 1.0);
    int noiseCommunity;
    double alphaValue = 1 - 1 / communityCount;
    unordered_map<int, int> sideInformation;
    for (const Node& node: c_ll.nodes) {
        if (prob_dist(gen) > alphaValue) {
            noiseCommunity = node.label;
        } else {
            noiseCommunity = dist(gen);
            if (noiseCommunity >= node.label) {
                noiseCommunity += 1;
            }
        }
        sideInformation.emplace(node.id, noiseCommunity);
    }

    for (Node& node: c_ll.nodes) {
        node.label = sideInformation.at(node.id);
    }
    for (Node& node: c_aux.nodes) {
        node.label = sideInformation.at(node.id);
    }
}

vector<pair<int, int>> DynamicCommunityDetection::oneLevel(Graph& auxiliary_graph) {
    unordered_map<int, vector<int>> communities;
    for (const Node& node: auxiliary_graph.nodes) {
        communities[node.label].push_back(node.id);
    }

    vector<pair<int, int>> changed_nodes{};
    vector<int> node_indices(auxiliary_graph.nodes.size());
    iota(node_indices.begin(), node_indices.end(), 0);
    mt19937 g(rd());
    shuffle(node_indices.begin(), node_indices.end(), g);
    double initial_modularity = modularity(auxiliary_graph);

    for (int idx: node_indices) {
        Node& node = auxiliary_graph.nodes[idx];

        int current_community = node.label;
        int best_community = current_community;
        double max_mod_gain = numeric_limits<double>::lowest();

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
            node.label = community;

            double mod_gain = modularity_gain(auxiliary_graph, node, current_community, community);

            if (mod_gain >= 0 && mod_gain > max_mod_gain) {
                best_community = community;
                max_mod_gain = mod_gain;
            }

            // Move the node back to its original community
            node.label = current_community;
        }

        // Move node to the best community found
        if (best_community != current_community) {
            if (node.id == current_community) {
                int newLabel = -1;
                for (int newLabelCandidate: communities.at(current_community)) {
                    if (newLabelCandidate != current_community) {
                        newLabel = newLabelCandidate;
                        break;
                    }
                }
                if (newLabel != -1) {
                    for (int movedNodeId: communities.at(current_community)) {
                        if (node.id == movedNodeId) {
                            continue;
                        }
                        Node& movedNode = auxiliary_graph.getNode(movedNodeId);
                        movedNode.label = newLabel;
                        communities[newLabel].push_back(movedNodeId);
                        changed_nodes.emplace_back(movedNodeId, newLabel);
                    }
                }
                communities.erase(current_community);
            } else {
                vector<int>& current_community_list = communities.at(current_community);
                current_community_list.erase(std::remove(current_community_list.begin(), current_community_list.end(), node.id), current_community_list.end());
            }
            node.label = best_community;
            communities[best_community].push_back(node.id);
            changed_nodes.emplace_back(node.id, best_community); // write all nodes to changed nodes
        }
    }

    return changed_nodes;
}

void DynamicCommunityDetection::updateCommunities(const vector<pair<int, int>>& changed_nodes) {
    unordered_map<int, vector<int>> communities;
    for (const Node& node: c_ll.nodes) {
        communities[node.label].push_back(node.id);
    }

    for (const auto& node_pair : changed_nodes) {
        for (int nodeId: communities.at(node_pair.first)) {
            Node& node = c_ll.getNode(nodeId);
            node.label = node_pair.second;
        }
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

void DynamicCommunityDetection::mergeCommunities() {
    unordered_map<int, vector<int>> communityMap{};
    for (const Node& node: c_ll.nodes) {
        communityMap[node.label].push_back(node.id);
    }
    vector<pair<int, vector<int>>> communityVector(communityMap.begin(), communityMap.end());
    sort(communityVector.begin(), communityVector.end(), [](const pair<int, vector<int>>& left, const pair<int, vector<int>>& right) {
        return left.second.size() > right.second.size();
    });

    int binCapacity = ceil(c_ll.nodes.size() / communityCount);
    vector<tuple<int, int, vector<int>>> binStore;

    int min, minIndex, binRemainingCapacity;
    for (auto community: communityVector) {
        int communitySize = community.second.size();

        // Add new separate entry if community size is already more than expected
        if (communitySize >= binCapacity) {
            binStore.emplace_back(0, community.first, community.second);
            continue;
        }

        min = binCapacity + 1;
        for (int i = 0; i < binStore.size(); ++i) {
            binRemainingCapacity = get<0>(binStore[i]);
            if (binRemainingCapacity >= communitySize && binRemainingCapacity - communitySize < min) {
                min = binRemainingCapacity - communitySize;
                minIndex = i;
            }
        }

        if (min == binCapacity + 1) {
            binStore.emplace_back(binCapacity - communitySize, community.first, community.second);
        } else {
            get<0>(binStore[minIndex]) -= communitySize;
            get<2>(binStore[minIndex]).insert(get<2>(binStore[minIndex]).end(), community.second.begin(), community.second.end());
        }
    }

    // Update the merged community labels
    for (auto bin: binStore) {
        int updatedLabel = get<1>(bin);
        for (int nodeId: get<2>(bin)) {
            Node& node = c_ll.getNode(nodeId);
            node.label = updatedLabel;
        }
    }
}

double DynamicCommunityDetection::modularity_gain(Graph& auxiliary_graph, const Node& node, int old_community, int new_community) {
    double mod_gain = 0.0;
    int m = auxiliary_graph.getTotalEdges();

    for (const auto& edge: node.edgeList) {
        const Node& neighbor = auxiliary_graph.getNode(get<1>(edge));
        int weight = get<2>(edge);
        if (neighbor.label == new_community) {
            mod_gain += (1.0 / (2.0 * m)) * (weight - ((double) node.edgeList.size() * (double) neighbor.edgeList.size()) / (2.0 * m));
        } else if (neighbor.label == old_community) {
            mod_gain -= (1.0 / (2.0 * m)) * (weight - ((double) node.edgeList.size() * (double) neighbor.edgeList.size()) / (2.0 * m));
        }
    }

    return mod_gain;
}
