#include "graph.h"
#include "color_map.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <map>

Node::Node(int id): id(id), label(-1), offset(-1), edgeList{}, messages{} {}

Node::~Node() {
    // Nothing to clean
}

Graph::Graph(int numberNodes) {
    for (int i = 0; i < numberNodes; ++i) {
        nodes.emplace_back(i);
        id_to_index_mapping.emplace(i, i);
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
    for (const auto& node: nodes) {
        Agnode_t *agnode_src = agnode_map[node.id];
        for (const auto& edge: node.edgeList) {
            int dest = get<1>(edge);
            // int weight = get<2>(edge);
            Agnode_t *agnode_dest = agnode_map[dest];
            Agedge_t *ag_edge = agedge(g, agnode_src, agnode_dest, NULL, 1);

            string edge_color = getEdgeColor(node.label, nodes[dest].label);

            // Set edge color
            agsafeset(ag_edge, const_cast<char*>("color"), const_cast<char*>(edge_color.c_str()), const_cast<char*>(""));
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

// Total edges includes weight as well
int Graph::getTotalEdges() const {
    int edgeCount = 0;
    for (const Node& node: nodes) {
        for (const auto& edge: node.edgeList) {
            edgeCount += get<2>(edge);
        }
    }
    return edgeCount / 2;
}

const Node& Graph::getNode(int nodeId) const {
    auto it = id_to_index_mapping.find(nodeId);
    if (it == id_to_index_mapping.end()) {
        throw out_of_range("Node with id " + to_string(nodeId) + " not found in id to index mapping.");
    }
    return nodes[it->second];
}

Node& Graph::getNode(int nodeId) {
    auto it = id_to_index_mapping.find(nodeId);
    if (it == id_to_index_mapping.end()) {
        throw out_of_range("Node with id " + to_string(nodeId) + " not found in id to index mapping.");
    }
    return nodes[it->second];
}

void Graph::addEdge(int srcNodeId, int destNodeId, int edgeWeight) {
    // Don't add edge entry if edge weight is 0
    if (edgeWeight == 0) {
        return;
    }

    Node& src = getNode(srcNodeId);
    auto it = find_if(src.edgeList.begin(), src.edgeList.end(),
                [&](const tuple<int, int, int>& edge) {
                    return get<1>(edge) == destNodeId;
                });
    if (it != src.edgeList.end()) {
        get<2>(*it) += edgeWeight;
    } else {
        src.edgeList.emplace_back(srcNodeId, destNodeId, edgeWeight);
    }
}

void Graph::addUndirectedEdge(int srcNodeId, int destNodeId, int edgeWeight) {
    addEdge(srcNodeId, destNodeId, edgeWeight);
    addEdge(destNodeId, srcNodeId, edgeWeight); // Since the graph is undirected
}

int Graph::getEdgeWeight(int srcNodeId, int destNodeId) {
    Node& src = getNode(srcNodeId);
    auto it = find_if(src.edgeList.begin(), src.edgeList.end(),
                [&](const tuple<int, int, int>& edge) {
                    return get<1>(edge) == destNodeId;
                });
    if (it == src.edgeList.end()) {
        return 0;
    }
    return get<2>(*it);
}

void Graph::removeEdge(int srcNodeId, int destNodeId) {
    Node& src = getNode(srcNodeId);
    auto it = find_if(src.edgeList.begin(), src.edgeList.end(),
                [&](const tuple<int, int, int>& edge) {
                    return get<1>(edge) == destNodeId;
                });

    if (it != src.edgeList.end()) {
        src.edgeList.erase(it);
    }
}

void Graph::removeUndirectedEdge(int srcNodeId, int destNodeId) {
    removeEdge(srcNodeId, destNodeId);
    // Since the graph is undirected
    if (srcNodeId != destNodeId) {
        removeEdge(destNodeId, srcNodeId);
    }
}

void Graph::addNode(int nodeId, int nodeLabel) {
    // Create node and push it to the node queue
    Node node = Node(nodeId);
    node.label = nodeLabel;
    nodes.push_back(node);

    // Update mappings
    int nodeIndex = id_to_index_mapping.size();
    id_to_index_mapping.emplace(nodeId, nodeIndex);
}

void Graph::removeNode(int nodeId) {
    try {
        int nodeIndex = id_to_index_mapping.at(nodeId);

        const Node& node = nodes[nodeIndex];
        // Remove edge entry from all neighbors
        for (const auto& edge: node.edgeList) {
            int targetNodeId = get<1>(edge);
            if (targetNodeId != nodeId) {
                removeEdge(targetNodeId, nodeId);
            }
        }

        // Remove node from list
        nodes.erase(nodes.begin() + nodeIndex);

        // Update mappings
        id_to_index_mapping.erase(nodeId);
        for (int i = nodeIndex; i < nodes.size(); ++i) {
            id_to_index_mapping[nodes[i].id] = i;
        }
    } catch (const out_of_range& e) {
        cerr << "Node with id " << nodeId << " not found in id to index mapping." << endl;
    } catch (const std::exception& e) {
        throw runtime_error("An error occurred while removing the node: " + string(e.what()));
    }
}

unordered_map<int, int> Graph::getLabels() {
    unordered_map<int, int> predicted_labels{};
    for (const auto& node: nodes) {
        predicted_labels.emplace(node.id, node.label);
    }

    return predicted_labels;
}
