#ifndef APPROXIMATE_COMMUNITY_DETECTION_H
#define APPROXIMATE_COMMUNITY_DETECTION_H

#include "src/graph.h"
#include "utils/quality_measures.h"
#include <random>
#include <unordered_set>
#include <numeric>

using namespace std;


class Community {
    private:
        struct EdgeComparator {
            bool operator()(const pair<int, double>& left, const pair<int, double>& right) {
                return left.second < right.second;  // Max-heap: higher weight comes first
            }
        };

    public:
        int e_in = 0;
        int e_out = 0;
        vector<Node*> nodes;
        // Nodes within the community with edges going out of the community
        priority_queue<pair<int, double>, vector<pair<int, double>>, EdgeComparator> nodes_to_be_removed;
        // Nodes out of the community with edges coming into the community
        priority_queue<pair<int, double>, vector<pair<int, double>>, EdgeComparator> nodes_to_be_added;
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
        bool nodeSwapAllowed(int community_label);
        double modularityContributionByCommunity(const Community& comm);
        double getModularity(int e_in, int e_out, int total_edges);

    public:
        ApproximateCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~ApproximateCommunityDetection();
};

#endif // APPROXIMATE_COMMUNITY_DETECTION_H
