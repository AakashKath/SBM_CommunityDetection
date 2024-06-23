#include "graph.h"
#include "color_map.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <map>

Node::Node(int id, vector<int> edgeList): id(id), label(-1), edgeList(edgeList) {}

Node::~Node() {
    // Nothing to clean
}

Graph::Graph(int numberNodes) {
    for (int i = 0; i < numberNodes; ++i) {
        nodes.emplace_back(i, vector<int>());
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
        if(subgraph_map.find(node.label) == subgraph_map.end()) {
            string subgraph_name = "cluster_" + to_string(node.label);
            Agraph_t *subgraph = agsubg(g, const_cast<char*>(subgraph_name.c_str()), 1);
            subgraph_map[node.label] = subgraph;
        }
    }

    // Add nodes to the subgraphs in Graphviz
    for (const auto &node: nodes) {
        string  node_name = to_string(node.id);
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
    for (const auto &src_node: nodes) {
        Agnode_t *agnode_src = agnode_map[src_node.id];
        for (int edge_dest: src_node.edgeList) {
            Agnode_t *agnode_dest = agnode_map[edge_dest];
            Agedge_t *edge = agedge(g, agnode_src, agnode_dest, NULL, 1);

            string edge_color = getEdgeColor(src_node.label, nodes[edge_dest].label);

            // Set edge color
            agsafeset(edge, const_cast<char*>("color"), const_cast<char*>(edge_color.c_str()), const_cast<char*>(""));
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
