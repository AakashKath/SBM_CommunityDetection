#include "graph.h"
#include "utils/color_map.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <map>

Node::Node(int id, int label): id(id), label(label), offset(-1), edgeList{}, messages{} {}

Node::~Node() {
    // Nothing to clean
}

void Node::addEdge(Node* destination, int edgeWeight) {
    // Don't add edge entry if edge weight is 0
    if (edgeWeight == 0) {
        return;
    }

    auto it = find_if(edgeList.begin(), edgeList.end(),
                [&](const pair<Node*, int>& edge) {
                    return edge.first->id == destination->id;
                });
    if (it != edgeList.end()) {
        it->second += edgeWeight;
    } else {
        edgeList.emplace_back(destination, edgeWeight);
    }
}

// Constructor to initialize graph with a certain number of nodes
Graph::Graph(int numberNodes) {
    for (int i = 0; i < numberNodes; ++i) {
        auto node = make_unique<Node>(i, i);
        Node* nodePtr = node.get();
        nodes.push_back(move(node));
        id_to_index_mapping.emplace(i, i);
    }
}

Graph::~Graph() {
    // Nothing to clean
}

// Move constructor
Graph::Graph(Graph&& other) noexcept
    : nodes(move(other.nodes)),
        id_to_index_mapping(move(other.id_to_index_mapping)) {}

// Move assignment operator
Graph& Graph::operator=(Graph&& other) noexcept {
    if (this != &other) {
        nodes = move(other.nodes);
        id_to_index_mapping = move(other.id_to_index_mapping);
    }
    return *this;
}

// Copy Constructor
Graph::Graph(const Graph& other) {
    // Deep copy each node
    for (const auto& node : other.nodes) {
        auto newNode = make_unique<Node>(node->id, node->label);
        newNode->offset = node->offset;
        newNode->messages = node->messages;
        nodes.push_back(move(newNode));
    }

    id_to_index_mapping = other.id_to_index_mapping;

    // Map old nodes to new nodes for correct edge assignment
    for (const auto& node: other.nodes) {
        for (const auto& edge: node->edgeList) {
            int srcId = node->id;
            int destId = edge.first->id;
            int edgeWeight = edge.second;
            Node* newSrcNode = getNode(srcId);
            Node* newDestNode = getNode(destId);
            if (newSrcNode && newDestNode) {
                newSrcNode->edgeList.emplace_back(newDestNode, edgeWeight);
            }
        }
    }
}

// Copy Assignment Operator
Graph& Graph::operator=(const Graph& other) {
    if (this != &other) {
        // Clear existing nodes and mapping
        nodes.clear();
        id_to_index_mapping.clear();

        // Deep copy each node
        for (const auto& node : other.nodes) {
            auto newNode = make_unique<Node>(node->id, node->label);
            newNode->offset = node->offset;
            newNode->messages = node->messages;
            nodes.push_back(move(newNode));
        }

        id_to_index_mapping = other.id_to_index_mapping;

        // Map old nodes to new nodes for correct edge assignment
        for (const auto& node: other.nodes) {
            for (const auto& edge: node->edgeList) {
                int srcId = node->id;
                int destId = edge.first->id;
                int edgeWeight = edge.second;
                Node* newSrcNode = getNode(srcId);
                Node* newDestNode = getNode(destId);
                if (newSrcNode && newDestNode) {
                    newSrcNode->edgeList.emplace_back(newDestNode, edgeWeight);
                }
            }
        }
    }
    return *this;
}

void Graph::draw(const string &filename) {
    // Create filepath
    string filepath = TEST_OUTPUT_DIRECTORY + filename;

    // Create a new graph
    Agraph_t *g = agopen(const_cast<char*>("g"), Agundirected, NULL);

    // Create a map to store subgraphs for each label
    map<int, Agraph_t*> subgraph_map;

    // Create a map to store the Agnode_t* for each node id
    map<int, Agnode_t*> agnode_map;

    // Create subgraphs for each label
    for (const auto& node: nodes) {
        if (subgraph_map.find(node->label) == subgraph_map.end()) {
            string subgraph_name = "cluster_" + to_string(node->label);
            Agraph_t *subgraph = agsubg(g, const_cast<char*>(subgraph_name.c_str()), 1);
            subgraph_map[node->label] = subgraph;
        }
    }

    // Add nodes to the subgraphs in Graphviz
    for (const auto &node: nodes) {
        string node_name = to_string(node->id);
        Agraph_t *subgraph = subgraph_map[node->label];
        Agnode_t *agnod = agnode(subgraph, const_cast<char*>(to_string(node->id).c_str()), 1);
        agnode_map[node->id] = agnod;

        string node_color = getNodeColor(node->label);

        // Set node color
        agsafeset(agnod, const_cast<char*>("color"), const_cast<char*>(node_color.c_str()), const_cast<char*>(""));
        agsafeset(agnod, const_cast<char*>("style"), const_cast<char*>("filled"), const_cast<char*>(""));
        agsafeset(agnod, const_cast<char*>("fillcolor"), const_cast<char*>(node_color.c_str()), const_cast<char*>(""));
    }

    // Create edges in Graphviz
    for (const auto& node: nodes) {
        Agnode_t *agnode_src = agnode_map[node->id];
        for (const auto& edge: node->edgeList) {
            int dest = edge.first->id;
            // int weight = get<2>(edge);
            Agnode_t *agnode_dest = agnode_map[dest];
            Agedge_t *ag_edge = agedge(g, agnode_src, agnode_dest, NULL, 1);

            string edge_color = getEdgeColor(node->label, nodes[dest]->label);

            // Set edge color
            agsafeset(ag_edge, const_cast<char*>("color"), const_cast<char*>(edge_color.c_str()), const_cast<char*>(""));
        }
    }

    // Create a Graphviz context
    GVC_t *gvc = gvContext();

    // Layout the graph
    gvLayout(gvc, g, "dot");

    // Render the graph to a file
    gvRenderFilename(gvc, g, "png", filepath.c_str());

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
    for (const auto& node: nodes) {
        for (const auto& edge: node->edgeList) {
            edgeCount += edge.second;
        }
    }
    return edgeCount / 2;
}

const Node* Graph::getNode(int nodeId) const {
    auto it = id_to_index_mapping.find(nodeId);
    if (it == id_to_index_mapping.end()) {
        throw out_of_range("Node with id " + to_string(nodeId) + " not found in id to index mapping.");
    }
    return nodes[it->second].get();
}

Node* Graph::getNode(int nodeId) {
    auto it = id_to_index_mapping.find(nodeId);
    if (it == id_to_index_mapping.end()) {
        throw out_of_range("Node with id " + to_string(nodeId) + " not found in id to index mapping.");
    }
    return nodes[it->second].get();
}

void Graph::addUndirectedEdge(Node* srcNode, Node* destNode, int edgeWeight) {
    srcNode->addEdge(destNode, edgeWeight);
    destNode->addEdge(srcNode, edgeWeight); // Since the graph is undirected
}

void Graph::addUndirectedEdge(int srcNodeId, int destNodeId, int edgeWeight) {
    Node* srcNode = getNode(srcNodeId);
    Node* destNode = getNode(destNodeId);
    addUndirectedEdge(srcNode, destNode);
}

int Graph::getEdgeWeight(int srcNodeId, int destNodeId) {
    Node* src = getNode(srcNodeId);
    auto it = find_if(src->edgeList.begin(), src->edgeList.end(),
                [&](const pair<Node*, int>& edge) {
                    return edge.first->id == destNodeId;
                });
    if (it == src->edgeList.end()) {
        return 0;
    }
    return it->second;
}

void Graph::removeEdge(int srcNodeId, int destNodeId) {
    Node* src = getNode(srcNodeId);
    auto it = find_if(src->edgeList.begin(), src->edgeList.end(),
                [&](const pair<Node*, int>& edge) {
                    return edge.first->id == destNodeId;
                });

    if (it != src->edgeList.end()) {
        src->edgeList.erase(it);
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
    auto node = make_unique<Node>(nodeId, nodeLabel);
    Node* nodePtr = node.get();
    nodes.push_back(move(node));

    // Update mappings
    int nodeIndex = id_to_index_mapping.size();
    id_to_index_mapping.emplace(nodeId, nodeIndex);
}

void Graph::removeNode(int nodeId) {
    try {
        int nodeIndex = id_to_index_mapping.at(nodeId);

        Node* node = nodes[nodeIndex].get();
        // Remove edge entry from all neighbors
        for (const auto& edge: node->edgeList) {
            Node* targetNode = edge.first;
            if (targetNode->id != nodeId) {
                removeEdge(targetNode->id, nodeId);
            }
        }

        // Remove node from list
        nodes.erase(nodes.begin() + nodeIndex);

        // Update mappings
        id_to_index_mapping.erase(nodeId);
        for (int i = nodeIndex; i < nodes.size(); ++i) {
            id_to_index_mapping[nodes[i]->id] = i;
        }
    } catch (const out_of_range& e) {
        cerr << "Node with id " << nodeId << " not found in id to index mapping." << endl;
    } catch (const exception& e) {
        throw runtime_error("An error occurred while removing the node: " + string(e.what()));
    }
}

unordered_map<int, int> Graph::getLabels() const {
    unordered_map<int, int> predicted_labels{};
    for (const auto& node: nodes) {
        predicted_labels.emplace(node->id, node->label);
    }

    return predicted_labels;
}

unordered_map<int, set<int>> Graph::getCommunities() const {
    unordered_map<int, set<int>> community_clusters{};
    for (const auto& node: nodes) {
        community_clusters[node->label].insert(node->id);
    }
    return community_clusters;
}
