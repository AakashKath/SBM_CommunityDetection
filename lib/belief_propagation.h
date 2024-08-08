#ifndef BELIEF_PROPAGATION_H
#define BELIEF_PROPAGATION_H

#include "graph.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>

using namespace std;


// BeliefPropagation class
class BeliefPropagation {
    public:
        BeliefPropagation(Graph graph, int communityCount, int impactRadius, double intra_community_edge_probability, double inter_community_edge_probability, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~BeliefPropagation();

        unordered_map<int, int> getCommunityLabels();

    private:
        Graph bp_graph;
        int impactRadius;
        int communityCount;
        double intra_community_edge_probability;
        double inter_community_edge_probability;
        double alphaValue;
        random_device rd;
        mt19937 gen;
        unordered_map<int, int> sideInformation;    // Side information for now is noise labels

        void processVertex(int nodeId, int involvedNeighborId);
        vector<double> StreamBP(const Node& node, vector<int> excludedNodeIds, int noiseLabel);
        unordered_map<int, vector<pair<int, int>>> collectRNeighborhood(int nodeId, int radius);
        double BP_0(int noiseLabel, int currentCommunity);
};

#endif // BELIEF_PROPAGATION_H
