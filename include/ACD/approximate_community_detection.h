#ifndef APPROXIMATE_COMMUNITY_DETECTION_H
#define APPROXIMATE_COMMUNITY_DETECTION_H

#include "src/graph.h"
#include "utils/quality_measures.h"
#include <random>
#include <unordered_set>
#include <numeric>
#include "utils/utilities.h"

using namespace std;


class ApproximateCommunityDetection {
    private:
        unordered_map<int, Community> communities;
        int communityCount;
        int totalEdges;
        mt19937 gen;

        void initialPartition();
        void createCommunities();
        void moveNodesForBestModularity();
        void swapNodes(int community_label);
        bool nodeSwapAllowed(int community_label);
        pair<Node*, Node*> addEdge(int srcId, int destId);
        void updateHeapAndMap(Node* node);
        void swapNodesIfPossible(int community_label);
        void repopulateHeapAndMap(Community& comm, int community_label);

    public:
        Graph acd_graph;

        ApproximateCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~ApproximateCommunityDetection();
};

#endif // APPROXIMATE_COMMUNITY_DETECTION_H
