#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <cmath>
#include "heap_and_map.h"
#include "src/graph.h"

using namespace std;


class Community {
    public:
        int e_in = 0;
        int e_out = 0;
        vector<Node*> nodes;
        // Nodes within the community with edges going out of the community
        HeapAndMap nodes_to_be_removed;
        // Nodes out of the community with edges coming into the community
        HeapAndMap nodes_to_be_added;

        Community();
        ~Community();
};

// Collection of Helper functions
double newmansModularity(unordered_map<int, Community> communities, int total_edges);
double newmansModularity(const Graph& graph);
double modularityContributionByCommunity(const Community& comm, int total_edges);
double getModularity(int e_in, int e_out, int total_edges);
double newmansModularity_(const Graph& graph, bool useSplitPenality = false, bool useDensity = false);

#endif // UTILITIES_H
