#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;


class Node {
    public:
        int id;
        int label;

        Node(int id);
        ~Node();
        int getLabel() const;
};

class Graph {
    public:
        Graph(int numberNodes);
        ~Graph();

        vector<Node> nodes;
        vector<vector<int>> adjacencyMatrix; // Represents edge weights, with 0 being no edge
        unordered_map<int, int> id_to_idx_mapping;

        void draw(const string &filename);
        string getEdgeColor(int srcLabel, int destLabel);
        string getNodeColor(int nodeLabel);
        int getTotalEdges();
        void addEdge(int srcNode, int destNode, int edgeWeight = 1);
        void removeEdge(int srcNode, int destNode);
        void addNode(int nodeId, int nodeLabel);
        void removeNode(int nodeId);
        bool hasNode(int nodeId);
        const Node& getNode(int nodeId) const;
        int getIdFromIdx(int idx);
};

#endif // GRAPH_H
