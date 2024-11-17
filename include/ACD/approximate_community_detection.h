#ifndef APPROXIMATE_COMMUNITY_DETECTION_H
#define APPROXIMATE_COMMUNITY_DETECTION_H

#include "src/graph.h"
#include "utils/quality_measures.h"
#include <random>
#include <unordered_set>

using namespace std;


class Community {
    private:
        struct EdgeComparator {
            bool operator()(const pair<int, int>& left, const pair<int, int>& right) {
                return left.second < right.second;  // Max-heap: higher weight comes first
            }
        };

    public:
        vector<Node*> nodes;
        // Nodes within the community with edges going out of the community
        priority_queue<pair<int, int>, vector<pair<int, int>>, EdgeComparator> out_edge_queue;
        // Nodes out of the community with edges coming into the community
        priority_queue<pair<int, int>, vector<pair<int, int>>, EdgeComparator> in_edge_queue;
};

class ApproximateCommunityDetection {
    private:
        Graph acd_graph;
        unordered_map<int, Community> communities;
        int communityCount;
        int totalEdges;
        mt19937 gen;

        void initialPartition();
        void createCommunities();
        void swapNodes(int community_label);

    public:
        ApproximateCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~ApproximateCommunityDetection();
};

#endif // APPROXIMATE_COMMUNITY_DETECTION_H
