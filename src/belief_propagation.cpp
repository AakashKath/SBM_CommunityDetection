#include "belief_propagation.h"


BeliefPropagation::BeliefPropagation(
    Graph graph,
    int communityCount,
    int impactRadius,
    double intra_community_edge_probability,
    double inter_community_edge_probability,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    bp_graph(graph),
    communityCount(communityCount),
    impactRadius(impactRadius),
    intra_community_edge_probability(intra_community_edge_probability),
    inter_community_edge_probability(inter_community_edge_probability),
    alphaValue(1 - 1 / communityCount)
{
    // Initialize noise as random numbers
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, communityCount - 2);
    uniform_real_distribution<double> prob_dist(0.0, 1.0);
    int noiseCommunity;
    for (const auto& node: bp_graph.nodes) {
        if (prob_dist(gen) > alphaValue) {
            noiseCommunity = node->label;
        } else {
            noiseCommunity = dist(gen);
            if (noiseCommunity >= node->label) {
                noiseCommunity += 1;
            }
        }
        sideInformation.emplace(node->id, noiseCommunity);
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
    Node* node = bp_graph.getNode(nodeId);

    if (node == nullptr) {
        return;
    }

    // Update incoming messsages for the new vertex
    for (const auto& edge: node->edgeList) {
        Node* neighbor = edge.first;

        // Skip if the neighbor was added in current iteration
        if (neighbor->id == involvedNeighborId) {
            continue;
        }

        neighbor->messages[nodeId] = StreamBP(neighbor, {nodeId, involvedNeighborId}, sideInformation.at(neighbor->id));
    }

    // Update outgoing messages up to `impactRadius` hops
    unordered_map<int, vector<pair<Node*, Node*>>> RNeighborhood = collectRNeighborhood(node, impactRadius);
    for (int radius = 1; radius <= impactRadius; ++radius) {
        if (RNeighborhood.find(radius) != RNeighborhood.end()) {
            for (const auto& [rNode, rParent]: RNeighborhood.at(radius)) {
                rParent->messages[rNode->id] = StreamBP(rParent, {nodeId, involvedNeighborId, rNode->id}, sideInformation.at(rParent->id));
            }
        } else {
            cout << "Radius " << radius << " not found in R-Neighborhood." << endl;
        }
    }
}

double BeliefPropagation::BP_0(int noiseLabel, int currentCommunity) const {
    return (alphaValue + (communityCount - 1 - communityCount * alphaValue) * (noiseLabel == currentCommunity)) / (communityCount - 1);
}

vector<double> BeliefPropagation::StreamBP(const Node* node, const vector<int> excludedNodeIds, int noiseLabel) {
    vector<double> result(communityCount, 1.0);

    for (const auto& edge: node->edgeList) {
        const Node* neighbor = edge.first;

        // Don't count excluded nodes in message re-calculation
        if (find(excludedNodeIds.begin(), excludedNodeIds.end(), neighbor->id) != excludedNodeIds.end()) {
            continue;
        }

        for (int s = 0; s < communityCount; ++s) {
            result[s] *= (inter_community_edge_probability + (intra_community_edge_probability - inter_community_edge_probability) * neighbor->messages.at(node->id)[s]) * BP_0(noiseLabel, s);
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

unordered_map<int, vector<pair<Node*, Node*>>> BeliefPropagation::collectRNeighborhood(Node* node, int radius) {
    unordered_map<int, vector<pair<Node*, Node*>>> neighborhood;
    unordered_set<int> visitedNodes;

    // Custom operator for priority queue to sort by distance
    auto cmp = [](const pair<Node*, int>& left, const pair<Node*, int>& right) {
        return left.second > right.second;
    };

    priority_queue<pair<Node*, int>, vector<pair<Node*, int>>, decltype(cmp)> distQueue(cmp);
    distQueue.emplace(node, 0);
    visitedNodes.insert(node->id);

    while (!distQueue.empty()) {
        auto [currentNode, currentDistance] = distQueue.top();
        distQueue.pop();

        if (currentNode == nullptr) {
            continue;
        }

        for (const auto& edge: currentNode->edgeList) {
            Node* nextNode = edge.first;

            // Skip loops
            if (visitedNodes.find(nextNode->id) != visitedNodes.end()) {
                continue;
            }

            int nextDistance = currentDistance + 1;
            if (nextDistance <= radius) {
                distQueue.emplace(nextNode, nextDistance);
                neighborhood[nextDistance].emplace_back(nextNode, currentNode);
                visitedNodes.insert(nextNode->id);
            }
        }
    }

    return neighborhood;
}

void BeliefPropagation::updateLabels() {
    for (auto& node: bp_graph.nodes) {
        vector<double> message = StreamBP(node.get(), {}, sideInformation.at(node->id));
        node->label = distance(message.begin(), max_element(message.begin(), message.end()));
    }
}
