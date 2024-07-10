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
        int numberCommunities;
        double intraCommunityEdgeProbability;
        double interCommunityEdgeProbability;
        vector<vector<int>> communityTracker;
        mt19937 gen;

        Graph generateSbm();
        bool isIntraCommunityEdge(double bias);
        pair<int, int> generateIntraCommunityEdge();
        pair<int, int> generateInterCommunityEdge();

    public:
        Sbm(int numberNodes, int numberCommunities, double intraCommunityEdgeProbability, double interCommunityEdgeProbability);
        ~Sbm();

        Graph sbm_graph;

        vector<int> listLabels();
        pair<int, int> generateEdge();
};

#endif // SBM_H
