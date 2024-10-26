#include "belief_propagation.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <cmath>


BeliefPropagation::BeliefPropagation(Graph graph, int communityCount, int impactRadius, double intra_community_edge_probability, double inter_community_edge_probability, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges): bp_graph(graph), communityCount(communityCount), impactRadius(impactRadius), intra_community_edge_probability(intra_community_edge_probability), inter_community_edge_probability(inter_community_edge_probability), alphaValue(1 - 1 / communityCount) {
    // Initialize noise as random numbers
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, communityCount - 2);
    uniform_real_distribution<double> prob_dist(0.0, 1.0);
    int noiseCommunity;
    for (const Node& node: bp_graph.nodes) {
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

    // Add edge and update corresponding message vector
    for (const auto& [node1Id, node2Id]: addedEdges) {
        // Skip self edges
        if (node1Id == node2Id) {
            continue;
        }
        bp_graph.addUndirectedEdge(node1Id, node2Id);
        processVertex(node1Id, node2Id);
        processVertex(node2Id, node1Id);
    }

    // Remove edge and update corresponding message vector
    for (const auto& [node1Id, node2Id]: removedEdges) {
        // Skip self edges
        if (node1Id == node2Id) {
            continue;
        }
        bp_graph.removeUndirectedEdge(node1Id, node2Id);
        processVertex(node1Id, node2Id);
        processVertex(node2Id, node1Id);
    }

    updateLabels();
}

BeliefPropagation::~BeliefPropagation() {
    // Nothing to clean
}

void BeliefPropagation::processVertex(int nodeId, int involvedNeighborId) {
    const Node& node = bp_graph.getNode(nodeId);

    // Update incoming messsages for the new vertex
    for (const auto& edge: node.edgeList) {
        int neighborId = get<1>(edge);

        // Skip if the neighbor was added in current iteration
        if (neighborId == involvedNeighborId) {
            continue;
        }

        Node& neighbor = bp_graph.getNode(neighborId);
        neighbor.messages[nodeId] = StreamBP(neighbor, {nodeId, involvedNeighborId}, sideInformation.at(neighborId));
    }

    // Update outgoing messages up to R hops
    unordered_map<int, vector<pair<int, int>>> RNeighborhood = collectRNeighborhood(nodeId, impactRadius);
    for (int radius = 1; radius <= impactRadius; ++radius) {
        if (RNeighborhood.find(radius) != RNeighborhood.end()) {
            for (const auto& [rNodeId, rParentId]: RNeighborhood.at(radius)) {
                Node& rParent = bp_graph.getNode(rParentId);
                rParent.messages[rNodeId] = StreamBP(rParent, {nodeId, involvedNeighborId, rNodeId}, sideInformation.at(rParentId));
            }
        } else {
            cout << "Radius " << radius << " not found in R-Neighborhood." << endl;
        }
    }
}

double BeliefPropagation::BP_0(int noiseLabel, int currentCommunity) {
    return (alphaValue + (communityCount - 1 - communityCount * alphaValue) * (noiseLabel == currentCommunity) ) / (communityCount - 1);
}

vector<double> BeliefPropagation::StreamBP(const Node& node, vector<int> excludedNodeIds, int noiseLabel) {
    vector<double> result(communityCount, 1.0);

    for (const auto& edge: node.edgeList) {
        int neighborId = get<1>(edge);

        // Don't count excluded nodes in message re-calculation
        if (find(excludedNodeIds.begin(), excludedNodeIds.end(), neighborId) != excludedNodeIds.end()) {
            continue;
        }

        const Node& neighbor = bp_graph.getNode(neighborId);
        for (int s = 0; s < communityCount; ++s) {
            result[s] *= (inter_community_edge_probability + (intra_community_edge_probability - inter_community_edge_probability) * neighbor.messages.at(node.id)[s]) * BP_0(noiseLabel, s);
        }
    }

    double Z = accumulate(result.begin(), result.end(), 0.0);
    if (Z != 0) {
        for (double& val: result) {
            val /= Z;
        }
    }

    return result;
}

unordered_map<int, vector<pair<int, int>>> BeliefPropagation::collectRNeighborhood(int nodeId, int radius) {
    unordered_map<int, vector<pair<int, int>>> neighborhood;
    unordered_set<int> visitedNodes;

    // Custom operator for priority queue to sort by distance
    auto cmp = [](const pair<int, int>& left, const pair<int, int>& right) {
        return left.second > right.second;
    };

    priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)> distQueue(cmp);
    distQueue.emplace(nodeId, 0);
    visitedNodes.emplace(nodeId);

    int currentDistance;
    int nextDistance;

    while(!distQueue.empty()) {
        pair<int, int> topElement = distQueue.top();
        int currentNodeId = topElement.first;
        const Node& node = bp_graph.getNode(currentNodeId);
        currentDistance = topElement.second;
        distQueue.pop();

        for (const auto& edge: node.edgeList) {
            int nextNodeId = get<1>(edge);

            // Skip loops
            if (visitedNodes.find(nextNodeId) != visitedNodes.end()) {
                continue;
            }

            int edgeWeight = get<2>(edge);
            nextDistance = currentDistance + edgeWeight;
            if (nextDistance <= radius) {
                distQueue.emplace(nextNodeId, nextDistance);
                neighborhood[nextDistance].emplace_back(nextNodeId, currentNodeId);
            }
        }
    }

    return neighborhood;
}

void BeliefPropagation::updateLabels() {
    vector<double> message;

    for (Node& node: bp_graph.nodes) {
        message = StreamBP(node, {}, sideInformation.at(node.id));
        node.label = distance(message.begin(), max_element(message.begin(), message.end()));
    }
}
