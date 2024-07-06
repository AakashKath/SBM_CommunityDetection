#include "belief_propagation.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <cmath>


BeliefPropagation::BeliefPropagation(Graph& graph, int communityCount, int impactRadius) : graph(graph), communityCount(communityCount), impactRadius(impactRadius) {}

BeliefPropagation::~BeliefPropagation() {
    // Nothing to clean
}

void BeliefPropagation::run() {
    for (int t = 0; t < graph.nodes.size(); ++t) {
        processVertex(t);
    }
}

void BeliefPropagation::processVertex(int nodeId) {
    const Node& node = graph.getNode(nodeId);

    // Update incoming messsages for the new vertex
    for (int w = 0; w < graph.nodes.size(); ++w) {
        if (graph.adjacencyMatrix[w][nodeId] > 0) {
            messages[w][nodeId] = StreamBP(messages[w], graph.getNode(w).bpLabel);
        }
    }

    // Update outgoing messages up to R hops
    for (int radius = 1; radius <= impactRadius; ++radius) {
        unordered_set<int> rNeighborhood = collectRNeighborhood(nodeId, radius);
        for (int v: rNeighborhood) {
            int v_prime = -1;
            double minDist = numeric_limits<double>::max();
            for (int w = 0; w < graph.nodes.size(); ++w) {
                if (graph.adjacencyMatrix[w][v] > 0 && messages[w].find(v) != messages[w].end()) {
                    double dist = graph.adjacencyMatrix[w][v];
                    if (dist < minDist) {
                        minDist = dist;
                        v_prime = w;
                    }
                }
            }
            if (v_prime != -1) {
                messages[v_prime][v] = StreamBP(messages[v_prime], graph.getNode(v_prime).bpLabel);
            }
        }
    }
}

vector<double> BeliefPropagation::StreamBP(const unordered_map<int, vector<double>>& incomingMessages, int noiseLabel) {
    vector<double> result(communityCount, 1.0);

    for(const auto& message: incomingMessages) {
        const vector<double>& m = message.second;
        for (int s = 0; s < communityCount; ++s) {
            result[s] *= (interCommProb + (intraCommProb - interCommProb) * m[s]);
        }
    }

    double Z = accumulate(result.begin(), result.end(), 0.0);
    for (double& val: result) {
        val /= Z;
    }

    return result;
}

vector<int> BeliefPropagation::getCommunityLabels() {
    vector<int> labels(graph.nodes.size());

    for (const auto& belief: beliefs) {
        int u = belief.first;
        const vector<double>& beliefVec = belief.second;
        labels[u] = distance(beliefVec.begin(), max_element(beliefVec.begin(), beliefVec.end()));
    }

    return labels;
}

unordered_set<int> BeliefPropagation::collectRNeighborhood(int nodeId, int radius) {
    unordered_set<int> neighborhood;
    queue<int> q;
    unordered_map<int, int> distances;
    q.push(nodeId);
    distances[nodeId] = 0;

    while(!q.empty()) {
        int current = q.front();
        q.pop();
        int currentDistance = distances[current];

        if (currentDistance < radius) {
            for (int w = 0; w < graph.nodes.size(); ++w) {
                if (graph.adjacencyMatrix[current][w] > 0 && distances.find(w) == distances.end()) {
                    q.push(w);
                    distances[w] = currentDistance + 1;
                    neighborhood.insert(w);
                }
            }
        }
    }

    return neighborhood;
}
