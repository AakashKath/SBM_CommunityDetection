#ifndef DYNAMIC_COMMUNITY_DETECTION_H
#define DYNAMIC_COMMUNITY_DETECTION_H

#include "src/graph.h"
#include "utils/quality_measures.h"
#include <numeric>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <tuple>

using namespace std;

typedef vector<unordered_set<int>> Communities;


class DynamicCommunityDetection {
    private:
        Graph c_ul;
        double mod, old_mod;
        int communityCount;

        void initialPartition(Graph& auxiliary_graph);
        vector<pair<int, int>> oneLevel(Graph& auxiliary_graph);
        void updateCommunities(const vector<pair<int, int>>& changed_nodes);
        void partitionToGraph();
        pair<pair<int, int>, unordered_set<int>> affectedByAddition(int src, int dest) const;
        pair<pair<int, int>, unordered_set<int>> affectedByRemoval(int src, int dest) const;
        void disbandCommunities(const unordered_set<int>& anodes);
        void syncCommunities(const pair<int, int>& involved_communities, const unordered_set<int>& anodes);
        void mergeCommunities();

    public:
        Graph c_ll;

        DynamicCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~DynamicCommunityDetection();
};

#endif // DYNAMIC_COMMUNITY_DETECTION_H
