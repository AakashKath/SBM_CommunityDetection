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
    double mod = newmansModularity(acd_graph, totalEdges);
    double old_mod = 0.0;
    do {
        old_mod = mod;
        mod = newmansModularity(acd_graph, totalEdges);
        uniform_int_distribution<int> community_randomiser(0, communityCount - 1);
        swapNodes(community_randomiser(gen));
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
        try {
            Community& comm = communities.at(node->label);
            comm.nodes.push_back(node.get());
        } catch (const out_of_range& e) {
            Community new_comm;
            new_comm.nodes.push_back(node.get());
            communities.emplace(node->label, move(new_comm));
        }
    }

    // Iterate through communities to fill out/in-edge priority queues
    for (auto& [community_label, comm]: communities) {
        unordered_map<int, int> incoming_edge_weight;

        // Fill out_edge_queue
        for (const auto& node: comm.nodes) {
            int outgoing_edge_weight = 0;
            for (const auto& edge: node->edgeList) {
                Node* neighbor = edge.first;
                int edge_weight = edge.second;

                if (neighbor->label != community_label) {
                    outgoing_edge_weight += edge_weight;
                    incoming_edge_weight[neighbor->id] += edge_weight;
                }
            }
            comm.out_edge_queue.emplace(node->id, outgoing_edge_weight);
        }

        // Fill in_edge_queue
        for (const auto& [neighbor_id, incoming_weight]: incoming_edge_weight) {
            comm.in_edge_queue.emplace(neighbor_id, incoming_weight);
        }
    }
}

void ApproximateCommunityDetection::swapNodes(int community_label) {
    try {
        Community& comm = communities.at(community_label);
        Node* node_eliminated = acd_graph.getNode(comm.out_edge_queue.top().first);
        Node* node_added = acd_graph.getNode(comm.in_edge_queue.top().first);
        comm.nodes.push_back(node_added);
        Community& involved_comm = communities.at(node_added->label);
        // remove node_added from involved_comm
    } catch (const out_of_range& e) {
        cout << "Community not found. No changes made." << endl;
    }
}
