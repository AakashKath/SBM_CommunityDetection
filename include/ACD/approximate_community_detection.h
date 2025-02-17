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
        int stopBefore;

        void initializePartition();
        void createCommunities();
        pair<Community&, Community&> addEdge(int srcId, int destId);
        void createHeapAndMap(Community& current_comm, Community& involved_comm);
        void run2FMAlgorithm(Community& comm1, Community& comm2);
        void updateCommunity(Community& comm, unordered_set<Node*> updated_comm_node_list);
        bool runKFMAlgorithm();
        void createKHeapAndMap();
        bool allCommunitiesQueueEmpty();
        Community* getMainCommunity();
        void updateKCommunityInformation(Node* node_moved, Community* main_community,
            Community* other_community, unordered_set<int>& frozen_node_ids);
        bool allCommunitiesSameSize();

    public:
        Graph acd_graph;

        ApproximateCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges, int stopBefore = -1, ofstream* outfile = nullptr);
        ~ApproximateCommunityDetection();
};

#endif // APPROXIMATE_COMMUNITY_DETECTION_H
