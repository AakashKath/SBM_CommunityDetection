#include "quality_measures.h"

double modularity(const Graph& auxiliary_graph) {
    int m = auxiliary_graph.getTotalEdges();

    // Modularity is 0 in case of no edges
    if (m == 0) {
        return 0.0;
    }

    double q = 0.0;
    unordered_map<int, int> degrees{};

    // Degree counts weight on edge as well
    for (const Node& node: auxiliary_graph.nodes) {
        int degreeWeight = 0;
        for (const auto& edge: node.edgeList) {
            degreeWeight += get<2>(edge);
        }
        degrees.emplace(node.id, degreeWeight);
    }

    for (const Node& srcNode: auxiliary_graph.nodes) {
        for (const auto& edge: srcNode.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            const Node& destNode = auxiliary_graph.getNode(destNodeId);
            try {
                if (srcNode.label == destNode.label) {
                    q += (weight - static_cast<double>(degrees.at(srcNode.id) * degrees.at(destNodeId)) / (2.0 * m));
                }
            } catch (const out_of_range& e) {
                throw out_of_range("Node " + to_string(srcNode.id) + " or " + to_string(destNodeId) + " not found.");
            }
        }
    }

    return q / (2.0 * m);
}
