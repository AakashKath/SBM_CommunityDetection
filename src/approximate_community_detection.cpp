#include "approximate_community_detection.h"


ApproximateCommunityDetection::ApproximateCommunityDetection(
    Graph graph,
    int communityCount,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    acd_graph(graph),
    communityCount(communityCount),
    totalEdges(graph.getTotalEdges()),
    gen(random_device{}())
{
    // Assign nodes to random community
    initialPartition();

    createCommunities();

    // TODO: Need to replace with change in modularity
    double old_mod = 0.0;
    double mod = newmansModularity(acd_graph);
    do {
        vector<int> randomised_communities(communityCount);
        iota(randomised_communities.begin(), randomised_communities.end(), 0); // Fill with 0, 1, ..., communityCount - 1
        shuffle(randomised_communities.begin(), randomised_communities.end(), gen);
        for (const auto& community_label: randomised_communities) {
            if (nodeSwapAllowed(community_label)) {
                swapNodes(community_label);
            }
        }
        old_mod = mod;
        mod = newmansModularity(acd_graph);
    } while (mod > old_mod);
}

ApproximateCommunityDetection::~ApproximateCommunityDetection() {
    // Nothing to clean
}

void ApproximateCommunityDetection::initialPartition() {
    int number_nodes = acd_graph.nodes.size();
    uniform_int_distribution<int> dist(0, number_nodes - 1);
    unordered_set<int> usedNumbers;
    int blockSize = number_nodes / communityCount;

    for (int i = 0; i < communityCount; ++i) {
        for (int j = 0; j < blockSize; ++j) {
            int num;
            do {
                num = dist(gen);
            } while (usedNumbers.find(num) != usedNumbers.end());

            // Update usedNumbers
            usedNumbers.insert(num);

            // Assign labels to vertices
            Node* node = acd_graph.getNode(num);
            node->label = i;
            node->offset = j;
        }
    }
}

void ApproximateCommunityDetection::createCommunities() {
    // Push node addresses into their respective communities
    for (const auto& node: acd_graph.nodes) {
        // Fetch or create relevant community
        Community& comm = [&]() -> Community& {
            try {
                return communities.at(node->label);
            } catch (const out_of_range&) {
                Community new_comm;
                return communities.emplace(node->label, move(new_comm)).first->second;
            }
        }();

        comm.nodes.push_back(node.get());
        for (const auto& edge: node.get()->edgeList) {
            if (edge.first->label == node->label) {
                comm.e_in += edge.second;
            } else {
                comm.e_out += edge.second;
            }
        }
    }

    for (auto& comm: communities) {
        comm.second.e_in /= 2;
    }

    // Iterate through communities to fill out/in-edge heapAndMap
    for (auto& [community_label, comm]: communities) {
        unordered_map<int, int> incoming_edge_weight;
        unordered_map<int, int> outgoing_edge_weight;

        for (const auto& node: comm.nodes) {
            for (const auto& edge: node->edgeList) {
                Node* neighbor = edge.first;
                int edge_weight = edge.second;
                if (neighbor->label != community_label) {
                    outgoing_edge_weight[node->id] += edge_weight;
                    incoming_edge_weight[neighbor->id] += edge_weight;
                }
            }
        }

        unordered_map<int, int> degrees{};
        // Degree counts weight on edge as well
        for (const auto& node: acd_graph.nodes) {
            int degreeWeight = 0;
            for (const auto& edge: node->edgeList) {
                degreeWeight += edge.second;
            }
            degrees.emplace(node->id, degreeWeight);
        }

        // Fill nodes_to_be_removed
        vector<pair<int, double>> outgoing_weight_vec;
        for (auto& [neighbor_id, outgoing_weight]: outgoing_edge_weight) {
            outgoing_weight_vec.emplace_back(neighbor_id, static_cast<double>(outgoing_weight) / degrees.at(neighbor_id));
        }
        comm.nodes_to_be_removed.populateHeapAndMap(outgoing_weight_vec);

        // Fill nodes_to_be_added
        vector<pair<int, double>> incoming_weight_vec;
        for (auto& [neighbor_id, incoming_weight]: incoming_edge_weight) {
            incoming_weight_vec.emplace_back(neighbor_id, static_cast<double>(incoming_weight) / degrees.at(neighbor_id));
        }
        comm.nodes_to_be_added.populateHeapAndMap(incoming_weight_vec);
    }
}

void ApproximateCommunityDetection::swapNodes(int community_label) {
    try {
        Community& comm = communities.at(community_label);

        // Get top elements from heapAndMap
        Node* node_removed = acd_graph.getNode(comm.nodes_to_be_removed.getMaxElementId());
        Node* node_added = acd_graph.getNode(comm.nodes_to_be_added.getMaxElementId());

        // Get the community of the added node
        int involved_comm_label = node_added->label;
        Community& involved_comm = communities.at(involved_comm_label);

        // Remove nodes from current community queues
        comm.nodes_to_be_removed.popElement();
        comm.nodes_to_be_added.popElement();

        // Remove nodes from involved community queues
        involved_comm.nodes_to_be_removed.deleteElement(node_added->id);
        involved_comm.nodes_to_be_added.deleteElement(node_removed->id);

        // Calculate updated weights for node_removed
        int old_community_edges = 0;
        int new_community_edges = 0;
        int degree = 0;
        for (const auto& edge: node_removed->edgeList) {
            if (edge.first->label != involved_comm_label) {
                new_community_edges += edge.second;
            }
            if (edge.first->label == community_label) {
                old_community_edges += edge.second;
            }
            degree += edge.second;
        }

        // Push the node into respective queues
        if (new_community_edges > 0) {
            involved_comm.nodes_to_be_removed.insertElement(node_removed->id, static_cast<double>(new_community_edges) / degree);
        }
        if (old_community_edges > 0) {
            comm.nodes_to_be_added.insertElement(node_removed->id, static_cast<double>(old_community_edges) / degree);
        }

        // Calculate updated weights for node_added
        old_community_edges = 0;
        new_community_edges = 0;
        degree = 0;
        for (const auto& edge: node_added->edgeList) {
            if (edge.first->label != community_label) {
                new_community_edges += edge.second;
            }
            if (edge.first->label == involved_comm_label) {
                old_community_edges += edge.second;
            }
            degree += edge.second;
        }

        // Push the node into respective queues
        if (old_community_edges > 0) {
            involved_comm.nodes_to_be_added.insertElement(node_added->id, static_cast<double>(old_community_edges) / degree);
        }
        if (new_community_edges > 0) {
            comm.nodes_to_be_removed.insertElement(node_added->id, static_cast<double>(new_community_edges) / degree);
        }

        // Update node communities
        node_added->label = community_label;
        node_removed->label = involved_comm_label;

        // Swap nodes among communities
        comm.nodes.erase(remove(comm.nodes.begin(), comm.nodes.end(), node_removed), comm.nodes.end());
        involved_comm.nodes.erase(remove(involved_comm.nodes.begin(), involved_comm.nodes.end(), node_added), involved_comm.nodes.end());
        comm.nodes.push_back(node_added);
        involved_comm.nodes.push_back(node_removed);
    } catch (const out_of_range& e) {
        cout << "Community not found. No changes made." << endl;
    }
}

bool ApproximateCommunityDetection::nodeSwapAllowed(int community_label) {
    try{
        Community& comm = communities.at(community_label);

        // Get top elements from heapAndMap
        Node* node_removed = acd_graph.getNode(comm.nodes_to_be_removed.getMaxElementId());
        Node* node_added = acd_graph.getNode(comm.nodes_to_be_added.getMaxElementId());

        // Get the community of the added node
        int involved_comm_label = node_added->label;
        Community& involved_comm = communities.at(involved_comm_label);

        double initial_modularity = modularityContributionByCommunity(comm, totalEdges) + modularityContributionByCommunity(involved_comm, totalEdges);

        int r_1_in{0}, r_1_out{0}, r_2_in{0}, r_2_out{0};
        for (const auto& edge: node_removed->edgeList) {
            // Ignore moving neighbor
            if (edge.first->id == node_added->id) {
                continue;
            }

            if (edge.first->label == community_label) {
                r_1_in += edge.second;
                r_1_out -= edge.second;
            } else {
                r_1_out += edge.second;
            }

            if (edge.first->label == involved_comm_label) {
                r_2_in += edge.second;
                r_2_out -= edge.second;
            } else {
                r_2_out += edge.second;
            }
        }

        int a_1_in{0}, a_1_out{0}, a_2_in{0}, a_2_out{0};
        for (const auto& edge: node_added->edgeList) {
            // Ignore moving neighbor
            if (edge.first->id == node_removed->id) {
                continue;
            }

            if (edge.first->label == community_label) {
                a_1_in += edge.second;
                a_1_out -= edge.second;
            } else {
                a_1_out += edge.second;
            }

            if (edge.first->label == involved_comm_label) {
                a_2_in += edge.second;
                a_2_out -= edge.second;
            } else {
                a_2_out += edge.second;
            }
        }

        double final_modularity = getModularity(comm.e_in - r_1_in + a_1_in, comm.e_out - r_1_out + a_1_out, totalEdges) +
            getModularity(involved_comm.e_in - a_2_in + r_2_in, involved_comm.e_out - a_2_out + r_2_out, totalEdges);

        if (final_modularity > initial_modularity) {
            return true;
        }
    } catch (const out_of_range& e) {
        cout << "Community not found. No changes made." << endl;
    }

    return false;
}
