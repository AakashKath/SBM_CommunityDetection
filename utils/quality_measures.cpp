#include "quality_measures.h"

double modularity(const Graph& graph) {
    int m = graph.getTotalEdges();

    // Modularity is 0 in case of no edges
    if (m == 0) {
        return 0.0;
    }

    double q = 0.0;
    unordered_map<int, int> degrees{};

    // Degree counts weight on edge as well
    for (const Node& node: graph.nodes) {
        int degreeWeight = 0;
        for (const auto& edge: node.edgeList) {
            degreeWeight += get<2>(edge);
        }
        degrees.emplace(node.id, degreeWeight);
    }

    for (const Node& srcNode: graph.nodes) {
        for (const auto& edge: srcNode.edgeList) {
            int destNodeId = get<1>(edge);
            int weight = get<2>(edge);
            const Node& destNode = graph.getNode(destNodeId);
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

int getSetDiff(unordered_set<int> predicted, unordered_set<int> original) {
    int diff_count = 0;
    for (const int & element: predicted) {
        if (original.find(element) == original.end()) {
            diff_count++;
        }
    }
    return diff_count;
}

int getCommonNodeCount(unordered_set<int> predicted, unordered_set<int> original) {
    int common_count = 0;
    for (const int& element: predicted) {
        if (original.find(element) != original.end()) {
            common_count++;
        }
    }
    return common_count;
}

double symmetricDifference(const Graph& graph, unordered_map<int, unordered_set<int>> original_labels) {
    int result = 0;
    unordered_set<int> predicted_label_set, original_label_set;
    unordered_map<int, unordered_set<int>> predicted_labels;
    for (const auto& node: graph.nodes) {
        predicted_labels[node.label].insert(node.id);
        predicted_label_set.insert(node.label);
    }
    for_each(original_labels.begin(), original_labels.end(), [&original_label_set](const auto& pair) {original_label_set.insert(pair.first);});

    // Custom operator for priority queue to sort by distance
    auto cmp = [](const tuple<int, int, int>& left, const tuple<int, int, int>& right) {
        return get<2>(left) < get<2>(right);
    };

    int max_common_label, max_common_value, common_value;
    tuple<int, int, int> max_mapping;
    for (int i = 0; i < predicted_labels.size(); ++i) {
        if (original_label_set.empty()) {
            cout << "No more mappings available." << endl;

            // Add remaining size
            for (const int& pKey: predicted_label_set) {
                result += predicted_labels.at(pKey).size();
            }

            // Return result as no more mapping can be made
            return result;
        }

        max_common_value = numeric_limits<int>::lowest();
        priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, decltype(cmp)> diffQueue(cmp);
        for (const int& pKey: predicted_label_set) {
            for (const int& oKey: original_label_set) {
                common_value = getCommonNodeCount(predicted_labels.at(pKey), original_labels.at(oKey));
                if (common_value > max_common_value) {
                    max_common_label = oKey;
                    max_common_value = common_value;
                }
            }
            diffQueue.emplace(pKey, max_common_label, max_common_value);
        }
        max_mapping = diffQueue.top();
        predicted_label_set.erase(get<0>(max_mapping));
        original_label_set.erase(get<1>(max_mapping));
        result += getSetDiff(predicted_labels.at(get<0>(max_mapping)), original_labels.at(get<1>(max_mapping)));
        diffQueue.pop();
    }

    return static_cast<double>(result) / graph.nodes.size();
}
