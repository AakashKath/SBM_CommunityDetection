#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>

using namespace std;


class Node {
    public:
        int id;
        int label;
        int offset;
        // TODO: need to store only address, all edge info will be stored in a very long list
        vector<pair<Node*, int>> edgeList; // {dest_address, weight}
        unordered_map<int, vector<double>> messages;

        Node(int id, int label = -1);
        ~Node();
        void addEdge(Node* destination, int edgeWeight = 1);
};

class Graph {
    public:
        // Constructors
        explicit Graph(int numberNodes);
        ~Graph();

        // Move constructor and assignment operator
        Graph(Graph&& other) noexcept;
        Graph& operator=(Graph&& other) noexcept;

        // Copy constructor and assignment operator
        Graph(const Graph& other);
        Graph& operator=(const Graph& other);

        vector<unique_ptr<Node>> nodes;
        unordered_map<int, size_t> id_to_index_mapping;

        void draw(const string &filename);
        string getEdgeColor(int srcLabel, int destLabel);
        string getNodeColor(int nodeLabel);
        int getTotalEdges() const;
        void addUndirectedEdge(Node* srcNode, Node* destNode, int edgeWeight = 1);
        void addUndirectedEdge(int srcNodeId, int destNodeId, int edgeWeight = 1);
        void removeEdge(int srcNodeId, int destNodeId);
        void removeUndirectedEdge(int srcNodeId, int destNodeId);
        int getEdgeWeight(int srcNodeId, int destNodeId);
        void addNode(int nodeId, int nodeLabel);
        void removeNode(int nodeId);
        const Node* getNode(int nodeId) const;
        Node* getNode(int nodeId);
        unordered_map<int, int> getLabels();
        unordered_map<int, unordered_set<int>> getCommunities();
};

#endif // GRAPH_H
