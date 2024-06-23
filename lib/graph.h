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
        vector<int> edgeList; // TODO: Change to Edge

        Node(int id, vector<int> edgeList);
        ~Node();
};

class Graph {
    public:
        Graph(int numberNodes);
        ~Graph();

        vector<Node> nodes;

        void draw(const string &filename);
        string getEdgeColor(int srcLabel, int destLabel);
        string getNodeColor(int nodeLabel);
};

#endif // GRAPH_H
