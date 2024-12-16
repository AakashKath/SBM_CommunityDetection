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

        void initializePartition();
        void createCommunities();
        pair<Community&, Community&> addEdge(int srcId, int destId);
        void createHeapAndMap(Community& current_comm, Community& involved_comm);
        void run2FMAlgorithm(Community& comm1, Community& comm2);
        void updateCommunity(Community& comm, unordered_set<Node*> updated_comm_node_list);

    public:
        Graph acd_graph;

        ApproximateCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~ApproximateCommunityDetection();
};

#endif // APPROXIMATE_COMMUNITY_DETECTION_H
