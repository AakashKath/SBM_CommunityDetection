#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <cmath>
#include "heap_and_map.h"
#include "src/graph.h"
#include "ortools/linear_solver/linear_solver.h"
#include <unordered_set>

using namespace std;
using namespace operations_research;


struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& pair) const {
        auto h1 = hash<T1>{}(pair.first);
        auto h2 = hash<T2>{}(pair.second);
        return h1 ^ h2;
    }
};

class Community {
    public:
        int id;
        int e_in = 0;
        int e_out = 0;
        unordered_set<Node*> nodes;
        // Node removal priority queue
        HeapAndMap node_removal_priority_queue;

        Community();
        ~Community();
};

// Collection of Helper functions
double newmansModularity(unordered_map<int, Community> communities, int total_edges);
double newmansModularity(const Graph& graph);
double modularityContributionByCommunity(const Community& comm, int total_edges);
double getModularity(int e_in, int e_out, int total_edges);
double newmansModularity_(const Graph& graph, bool useSplitPenality = false, bool useDensity = false);
void solve_ilp(const Graph& graph);

#endif // UTILITIES_H
