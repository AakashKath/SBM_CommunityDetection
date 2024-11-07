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
        int communityCount;
        int totalEdges;
        random_device rd;
        double epsilon_gain = 0.0001;

        void initialPartition(Graph& auxiliary_graph);
        vector<pair<int, int>> oneLevel(Graph& auxiliary_graph);
        void updateCommunities(const vector<pair<int, int>>& changed_nodes);
        void partitionToGraph();
        pair<pair<Node*, Node*>, unordered_set<Node*>> affectedByAddition(int src, int dest);
        pair<pair<Node*, Node*>, unordered_set<Node*>> affectedByRemoval(int src, int dest);
        void disbandCommunities(unordered_set<Node*>& anodes);
        void syncCommunities(pair<Node*, Node*>& involved_communities, unordered_set<Node*>& anodes);
        // void mergeCommunities();
        double modularity_gain(const Node* node, int new_community, int old_community);
        void relabelGraph();

    public:
        Graph c_ll;

        DynamicCommunityDetection(Graph graph, int communityCount, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~DynamicCommunityDetection();
};

#endif // DYNAMIC_COMMUNITY_DETECTION_H
