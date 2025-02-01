#ifndef HUNGARIAN_ALGORITHM_H
#define HUNGARIAN_ALGORITHM_H

#include <iostream>
#include <vector>
#include <queue>
#include <climits>

using namespace std;

class HungarianAlgorithm {
    private:
        vector<vector<int>> costMatrix; // Cost matrix representing workers vs jobs
        int size, matches; // Number of workers/jobs and current match count

        // Arrays for tracking the alternating tree used in augmenting paths
        vector<int> parent;  // To reconstruct augmenting paths
        vector<int> inTreeX, inTreeY;  // Tracks workers (X) and jobs (Y) in the tree

        // Labels for workers and jobs
        vector<int> labelX, labelY;  

        // Matching arrays: 
        // matchX[x] - job assigned to worker x, 
        // matchY[y] - worker assigned to job y
        vector<int> matchX, matchY;

        // Slack arrays used in label updates
        vector<int> slack, slackX;

        void initializeLabels();
        void augment();
        void initializeBFS(int &root, queue<int> &bfsQueue);
        void initializeSlack(int root);
        bool findAugmentingPath(queue<int> &bfsQueue, int &worker, int &job);
        void addToTree(int worker, int prevX);
        void updateLabels();
        bool extendAlternatingTree(queue<int> &bfsQueue, int &worker, int &job);
        void updateMatching(int worker, int job);
        void printMatching();

    public:
        HungarianAlgorithm(vector<vector<int>> costs);
        ~HungarianAlgorithm();

        int solveAssignmentProblem();
};

#endif // HUNGARIAN_ALGORITHM_H
