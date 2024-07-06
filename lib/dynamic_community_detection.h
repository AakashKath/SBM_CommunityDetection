#ifndef DYNAMIC_COMMUNITY_DETECTION_H
#define DYNAMIC_COMMUNITY_DETECTION_H

#include "graph.h"
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
        Graph& graph;
        Graph c_ll = Graph(0);
        Graph c_ul = Graph(0);
        Graph c_aux = Graph(0);
        vector<pair<int, int>> addedEdges, removedEdges;
        double mod, old_mod;

        void init();
        void initialPartition();
        double modularity(Graph& auxiliary_graph);
        vector<pair<int, int>> oneLevel();
        void updateCommunities(const vector<pair<int, int>>& changed_nodes);
        void partitionToGraph();
        tuple<pair<int, int>, vector<int>> affectedByAddition(int src, int dest);
        tuple<pair<int, int>, vector<int>> affectedByRemoval(int src, int dest);
        void addEdge(int src, int dest);
        void removeEdge(int src, int dest);
        void disbandCommunities(const vector<int>& anodes);
        void syncCommunities(const pair<int, int>& involved_communities, const vector<int>& anodes);

    public:
        DynamicCommunityDetection(Graph& graph, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~DynamicCommunityDetection();

        void run();
        unordered_map<int, int> getPredictedLabels();
};

#endif // DYNAMIC_COMMUNITY_DETECTION_H
