#ifndef SBM_H
#define SBM_H

#include <iostream>
#include <vector>

#include "graph.h"

using namespace std;

class Sbm {
    private:
        int numberNodes;
        int numberCommunities;
        vector<double> p;
        vector<vector<double>> W;

        void generateProbabilityDistribution();
        void generateProbabilityMatrix();
        Graph generateSbm();

    public:
        Sbm(int numberNodes, int numberCommunities);
        ~Sbm();

        Graph sbm_graph;

        vector<int> listLabels();
};

#endif // SBM_H
