#ifndef SBM_H
#define SBM_H

#include <iostream>
#include <vector>
#include <random>

#include "graph.h"

using namespace std;

class Sbm {
    private:
        int numberNodes;
        double communityBoundaryThreshold;
        vector<vector<int>> communityTracker;
        mt19937 gen;

        Graph generateSbm();
        bool isIntraCommunityEdge();
        pair<int, int> generateIntraCommunityEdge();
        pair<int, int> generateInterCommunityEdge();

    public:
        Sbm(int numberNodes, int numberCommunities, double intraCommunityEdgeProbability, double interCommunityEdgeProbability);
        ~Sbm();

        // Move constructor and assignment operator
        Sbm(Sbm&& other) noexcept;
        Sbm& operator=(Sbm&& other) noexcept;

        // Deleted copy constructor and assignment operator
        Sbm(const Sbm&) = delete;
        Sbm& operator=(const Sbm&) = delete;

        Graph sbm_graph;
        int numberCommunities;
        double intraCommunityEdgeProbability;
        double interCommunityEdgeProbability;

        pair<int, int> generateEdge();
};

#endif // SBM_H
