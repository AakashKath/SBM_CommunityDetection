#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using namespace std;


class Node {
    public:
        int id;
        int label;
        int offset;
        // TODO: need to store only address, all edge info will be stored in a very long list
        vector<tuple<int, int, int>> edgeList; // {src, dest, weight}
        unordered_map<int, vector<double>> messages;

        Node(int id);
        ~Node();
};

class Graph {
    public:
        Graph(int numberNodes);
        ~Graph();

        vector<Node> nodes;
        unordered_map<int, size_t> id_to_index_mapping;

        void draw(const string &filename);
        string getEdgeColor(int srcLabel, int destLabel);
        string getNodeColor(int nodeLabel);
        int getTotalEdges() const;
        void addEdge(int srcNodeId, int destNodeId, int edgeWeight = 1);
        void addUndirectedEdge(int srcNodeId, int destNodeId, int edgeWeight = 1);
        void removeEdge(int srcNodeId, int destNodeId);
        void removeUndirectedEdge(int srcNodeId, int destNodeId);
        int getEdgeWeight(int srcNodeId, int destNodeId);
        void addNode(int nodeId, int nodeLabel);
        void removeNode(int nodeId);
        const Node& getNode(int nodeId) const;
        Node& getNode(int nodeId);
        unordered_map<int, int> getLabels();
        unordered_map<int, unordered_set<int>> getCommunities();
};

#endif // GRAPH_H
