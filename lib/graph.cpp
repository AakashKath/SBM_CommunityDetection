#include "graph.h"
#include "color_map.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <map>

Node::Node(int id): id(id), label(-1) {}

Node::~Node() {
    // Nothing to clean
}

int Node::getLabel() const {
    return label;
}

Graph::Graph(int numberNodes): adjacencyMatrix(numberNodes, vector<int>(numberNodes, 0)) {
    for (int i = 0; i < numberNodes; ++i) {
        nodes.emplace_back(i);
    }
}

Graph::~Graph() {
    // Nothing to clean
}

void Graph::draw(const string &filename) {
    // Create a new graph
    Agraph_t *g = agopen(const_cast<char*>("g"), Agundirected, NULL);

    // Create a map to store subgraphs for each label
    map<int, Agraph_t*> subgraph_map;

    // Create a map to store the Agnode_t* for each node id
    map<int, Agnode_t*> agnode_map;

    // Create subgraphs for each label
    for (const auto &node: nodes) {
        if (subgraph_map.find(node.label) == subgraph_map.end()) {
            string subgraph_name = "cluster_" + to_string(node.label);
            Agraph_t *subgraph = agsubg(g, const_cast<char*>(subgraph_name.c_str()), 1);
            subgraph_map[node.label] = subgraph;
        }
    }

    // Add nodes to the subgraphs in Graphviz
    for (const auto &node: nodes) {
        string node_name = to_string(node.id);
        Agraph_t *subgraph = subgraph_map[node.label];
        Agnode_t *agnod = agnode(subgraph, const_cast<char*>(to_string(node.id).c_str()), 1);
        agnode_map[node.id] = agnod;

        string node_color = getNodeColor(node.label);

        // Set node color
        agsafeset(agnod, const_cast<char*>("color"), const_cast<char*>(node_color.c_str()), const_cast<char*>(""));
        agsafeset(agnod, const_cast<char*>("style"), const_cast<char*>("filled"), const_cast<char*>(""));
        agsafeset(agnod, const_cast<char*>("fillcolor"), const_cast<char*>(node_color.c_str()), const_cast<char*>(""));
    }

    // Create edges in Graphviz
    for (int i = 0; i < adjacencyMatrix.size(); ++i) {
        Agnode_t *agnode_src = agnode_map[i];
        for (int j = i + 1; j < adjacencyMatrix[i].size(); ++j) {
            if (adjacencyMatrix[i][j] > 0) {
                Agnode_t *agnode_dest = agnode_map[j];
                Agedge_t *edge = agedge(g, agnode_src, agnode_dest, NULL, 1);

                string edge_color = getEdgeColor(nodes[i].label, nodes[j].label);

                // Set edge color
                agsafeset(edge, const_cast<char*>("color"), const_cast<char*>(edge_color.c_str()), const_cast<char*>(""));
            }
        }
    }

    // Create a Graphviz context
    GVC_t *gvc = gvContext();

    // Layout the graph
    gvLayout(gvc, g, "dot");

    // Render the graph to a file
    gvRenderFilename(gvc, g, "png", filename.c_str());

    // Free the layout and the graph
    gvFreeLayout(gvc, g);
    agclose(g);

    // Free the context
    gvFreeContext(gvc);
}

string Graph::getEdgeColor(int srcLabel, int destLabel) {
    if (srcLabel == destLabel) {
        return "#00FF00"; // Green color in hex
    } else {
        return "#FF0000"; // Red color in hex
    }
}

string Graph::getNodeColor(int nodeLabel) {
    if (colorMap.find(nodeLabel) != colorMap.end()) {
        return colorMap[nodeLabel];
    } else {
        // Default color if the label is out of range
        return "#808080"; // Grey color in hex
    }
}

int Graph::getTotalEdges() {
    int edgeCount = 0;
    for (const auto &row: adjacencyMatrix) {
        for (int val: row) {
            edgeCount += val;
        }
    }
    return edgeCount/2;
}

const Node& Graph::getNode(int nodeId) const {
    return nodes[nodeId];
}

void Graph::addEdge(int srcNode, int destNode, int edgeWeight) {
    // TODO: Need to probability randomness
    adjacencyMatrix[srcNode][destNode] += edgeWeight;
    adjacencyMatrix[destNode][srcNode] += edgeWeight; // Since the graph is undirected
}

void Graph::removeEdge(int srcNode, int destNode) {
    // TODO: same as addEdge
    adjacencyMatrix[srcNode][destNode] = 0;
    adjacencyMatrix[destNode][srcNode] = 0; // Since the graph is undirected
}

void Graph::addNode(int nodeId, int nodeLabel) {
    Node node = Node(nodeId);
    node.label = nodeLabel;
    nodes.push_back(node);

    // Update the edge matrix to reflect edges of the new node
    adjacencyMatrix.resize(nodes.size(), vector<int>(nodes.size(), 0));
    for (int i = 0; i < nodes.size(); ++i) {
        adjacencyMatrix[i].resize(nodes.size(), 0);
    }

    // Adjust id to index mapping
    for (int i = 0; i < nodes.size(); ++i) {
        id_to_idx_mapping[nodes[i].id] = i;
    }
}

void Graph::removeNode(int nodeId) {
    // Remove node from list
    nodes.erase(nodes.begin() + nodeId);

    // Remove edge row
    adjacencyMatrix.erase(adjacencyMatrix.begin() + nodeId);

    // Remove edge column
    for (auto& row: adjacencyMatrix) {
        row.erase(row.begin() + nodeId);
    }

    // Adjust id to index mapping
    for (int i = 0; i < nodes.size(); ++i) {
        id_to_idx_mapping[nodes[i].id] = i;
    }
}

bool Graph::hasNode(int nodeId) {
    for (const Node& existing_node: nodes) {
        if (existing_node.id == nodeId) {
            return true;
        }
    }
    return false;
}

int Graph::getIdFromIdx(int idx) {
    for (auto& pair: id_to_idx_mapping) {
        if (pair.second == idx) {
            return pair.first;
        }
    }
    return -1;
}
