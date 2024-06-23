#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;


class Node {
    public:
        int id;
        int label;

        Node(int id);
        ~Node();
};

class Graph {
    public:
        Graph(int numberNodes);
        ~Graph();

        vector<Node> nodes;
        vector<vector<int>> adjacencyMatrix;

        void draw(const string &filename);
        string getEdgeColor(int srcLabel, int destLabel);
        string getNodeColor(int nodeLabel);
        int getTotalEdges();
};

#endif // GRAPH_H
