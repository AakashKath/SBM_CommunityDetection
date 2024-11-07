#ifndef BELIEF_PROPAGATION_H
#define BELIEF_PROPAGATION_H

#include "src/graph.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <queue>
#include <numeric>
#include <cmath>

using namespace std;


// BeliefPropagation class
class BeliefPropagation {
    public:
        Graph bp_graph;

        BeliefPropagation(Graph graph, int communityCount, int impactRadius, double intra_community_edge_probability, double inter_community_edge_probability, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~BeliefPropagation();

    private:
        int impactRadius;
        int communityCount;
        double intra_community_edge_probability;
        double inter_community_edge_probability;
        double alphaValue;
        random_device rd;
        mt19937 gen;
        unordered_map<int, int> sideInformation;    // Side information for now is noise labels

        void processVertex(int nodeId, int involvedNeighborId);
        vector<double> StreamBP(const Node* node, const vector<int> excludedNodeIds, int noiseLabel);
        unordered_map<int, vector<pair<Node*, Node*>>> collectRNeighborhood(Node* node, int radius);
        double BP_0(int noiseLabel, int currentCommunity) const;
        void updateLabels();
};

#endif // BELIEF_PROPAGATION_H
