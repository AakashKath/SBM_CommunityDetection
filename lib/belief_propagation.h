#ifndef BELIEF_PROPAGATION_H
#define BELIEF_PROPAGATION_H

#include "graph.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;


// BeliefPropagation class
class BeliefPropagation {
    public:
        BeliefPropagation(Graph graph, int communityCount, int impactRadius, double intra_community_edge_probability, double inter_community_edge_probability);
        ~BeliefPropagation();

        vector<int> getCommunityLabels();

    private:
        Graph bp_graph = Graph(0);
        int impactRadius;
        int communityCount;
        double intra_community_edge_probability;
        double inter_community_edge_probability;
        unordered_map<int, unordered_map<int, vector<double>>> messages;
        unordered_map<int, vector<double>> beliefs;

        void processVertex(int nodeId);
        vector<double> StreamBP(const unordered_map<int, vector<double>>& incomingMessages, int noiseLabel);
        unordered_set<int> collectRNeighborhood(int nodeId, int radius);
};

#endif // BELIEF_PROPAGATION_H
