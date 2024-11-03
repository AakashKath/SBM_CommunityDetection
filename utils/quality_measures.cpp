#include "quality_measures.h"

double modularity(const Graph& graph, int totalEdges) {
    if (totalEdges == -1) {
        cerr << "Calculating total edges, please see if this can be computed in the beginning and passed as a parameter." << endl;
        totalEdges = graph.getTotalEdges();
    }

    // Modularity is 0 in case of no edges
    if (totalEdges == 0) {
        return 0.0;
    }

    double q = 0.0;
    unordered_map<int, int> degrees{};

    // Degree counts weight on edge as well
    for (const auto& node: graph.nodes) {
        int degreeWeight = 0;
        for (const auto& edge: node->edgeList) {
            degreeWeight += edge.second;
        }
        degrees.emplace(node->id, degreeWeight);
    }

    for (const auto& srcNode: graph.nodes) {
        for (const auto& edge: srcNode->edgeList) {
            const Node* destNode = edge.first;
            int weight = edge.second;
            try {
                if (srcNode->label == destNode->label) {
                    q += (weight - static_cast<double>(degrees.at(srcNode->id) * degrees.at(destNode->id)) / (2.0 * totalEdges));
                }
            } catch (const out_of_range& e) {
                throw out_of_range("Node " + to_string(srcNode->id) + " or " + to_string(destNode->id) + " not found.");
            }
        }
    }

    return q / (2.0 * totalEdges);
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
        predicted_labels[node->label].insert(node->id);
        predicted_label_set.insert(node->label);
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

vector<size_t> get_cpu_times() {
    ifstream proc_stat("/proc/stat");
    proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
    vector<size_t> times;
    for (size_t time; proc_stat >> time; times.push_back(time));
    return times;
}

bool get_cpu_times(size_t &idle_time, size_t &total_time) {
    const vector<size_t> cpu_times = get_cpu_times();
    if (cpu_times.size() < 4)
        return false;
    idle_time = cpu_times[3];
    total_time = accumulate(cpu_times.begin(), cpu_times.end(), 0);
    return true;
}

long getRAMUsage() {
    ifstream status_file("/proc/self/status");
    string line;
    long ram_kb = 0;

    while (getline(status_file, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            ram_kb = stol(line.substr(line.find_last_of("\t") + 1));
            break;
        }
    }

    return ram_kb;
}

double f1Score(const Graph& graph, unordered_map<int, int> original_labels) {
    int true_positive = 0;
    int false_positive = 0;
    int false_negative = 0;

    /*
    We can have two kinds of edge, inter- and intra-community edges. Label assignment can be defined as follows
        1 = intra-community edge
        0 = inter-community edge
    For confusion matrix we define (Ground-truth label, Predicted label):
        True Positives: The number of edges correctly predicted as intra-community. (1, 1)
        True Negatives: The number of edges corerctly predicted as inter-community. (0, 0)
        False Positives: The number of edges incorrectly predicted as intra-community. (0, 1)
        False Negatives: The number of edges incorrectly predicted as inter-community. (1, 0)
    We define:
        Precision: The proportion of correctly predicted intra-community edges out of all edges predicted as
            intra-community. TP/(TP+FP)
        Recall (sensitivity): The proportion of correctly predicted intra-community edges out of all actual
            intra-community edges. TP/(TP+FN)
        F1Score: Harmonic mean of precision and recall.
    */

    for (const auto& node1: graph.nodes) {
        for (const auto& node2: graph.nodes) {
            if (node1->id == node2->id) {
                continue;
            }
            if (node1->label == node2->label && original_labels.at(node1->id) == original_labels.at(node2->id)) {
                true_positive++;
            } else if (original_labels.at(node1->id) == original_labels.at(node2->id) && node1->label != node2->label) {
                false_negative++;
            } else if (node1->label == node2->label && original_labels.at(node1->id) != original_labels.at(node2->id)) {
                false_positive++;
            }
        }
    }

    double precision = static_cast<double>(true_positive) / (true_positive + false_positive);
    double recall = static_cast<double>(true_positive) / (true_positive + false_negative);
    return 2 * precision * recall / (precision + recall);
}

double loglikelihood(const Graph& graph, const Graph& edgeGraph) {
    double loglikelihood = 0.0;
    int intraCommunityEdges = 0;
    int interCommunityEdges = 0;
    int intraCommunityPair = 0;
    int interCommunityPair = 0;
    for (const auto& edgeNode: edgeGraph.nodes) {
        for (const auto& edge: edgeNode->edgeList) {
            const Node* neighbor = edge.first;
            if (edgeNode->label == neighbor->label) {
                intraCommunityEdges++;
            } else {
                interCommunityEdges++;
            }
        }
    }
    intraCommunityEdges /= 2;
    interCommunityEdges /= 2;

    for (const auto& node1: graph.nodes) {
        for (const auto& node2: graph.nodes) {
            if (node1->id == node2->id) {
                continue;
            }
            if (node1->label == node2->label) {
                intraCommunityPair++;
            } else {
                interCommunityPair++;
            }
        }
    }
    intraCommunityPair /= 2;
    interCommunityPair /= 2;

    double estimatedIntraCommunityEdgeProbability = 0.0;
    double estimatedInterCommunityEdgeProbability = 0.0;
    if (intraCommunityPair > 0) {
        estimatedIntraCommunityEdgeProbability = (double)intraCommunityEdges / intraCommunityPair;
    }
    if (interCommunityPair > 0) {
        estimatedInterCommunityEdgeProbability = (double)interCommunityEdges / interCommunityPair;
    }

    if (estimatedIntraCommunityEdgeProbability != 0) {
        loglikelihood += intraCommunityEdges * log(estimatedIntraCommunityEdgeProbability) + (intraCommunityPair - intraCommunityEdges) * log(1.0 - estimatedIntraCommunityEdgeProbability);
    }
    if (estimatedInterCommunityEdgeProbability != 0) {
        loglikelihood += interCommunityEdges * log(estimatedInterCommunityEdgeProbability) + (interCommunityPair - interCommunityEdges) * log(1.0 - estimatedInterCommunityEdgeProbability);
    }

    return loglikelihood;
}

double embeddedness(const Graph& graph) {
    int withinCommunityNodes;
    double total_embeddedness = 0.0;
    for (const auto& node: graph.nodes) {
        withinCommunityNodes = 0;
        for (const auto& edge: node->edgeList) {
            const Node* neighbor = edge.first;
            if (node->label == neighbor->label) {
                withinCommunityNodes++;
            }
        }
        if (node->edgeList.size() > 0) {
            total_embeddedness += static_cast<double>(withinCommunityNodes) / node->edgeList.size();
        }
    }
    return total_embeddedness;
}

double accuracy(const Graph& graph, unordered_map<int, int> original_labels, int numberCommunities) {
    double max_accuracy = 0.0;
    vector<int> perm(numberCommunities);
    iota(perm.begin(), perm.end(), 0); // Fill with 0, 1, 2, ..., k-1

    // Iterate over all permutations of the label set
    do {
        int correct_count = 0;

        // Loop over all vertices
        for (const auto& node: graph.nodes) {
            int estimated_label = node->label; // Get the expected label
            int true_label = perm[original_labels.at(node->id)]; // Get the permuted true label

            if (estimated_label == true_label) {
                correct_count++; // Count correct matches
            }
        }

        // Compute accuracy for this permutation
        double accuracy = static_cast<double>(correct_count) / graph.nodes.size();
        if (accuracy > max_accuracy) {
            max_accuracy = accuracy; // Keep track of the best accuracy
        }

    } while (next_permutation(perm.begin(), perm.end())); // Generate next permutation

    return max_accuracy;
}
