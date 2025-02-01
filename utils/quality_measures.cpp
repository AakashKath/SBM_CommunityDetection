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

int getSetDiff(set<int> predicted, set<int> original) {
    int diff_count = 0;
    for (const int & element: predicted) {
        if (original.find(element) == original.end()) {
            diff_count++;
        }
    }
    return diff_count;
}

int getCommonNodeCount(set<int> predicted, set<int> original) {
    int common_count = 0;
    for (const int& element: predicted) {
        if (original.find(element) != original.end()) {
            common_count++;
        }
    }
    return common_count;
}

// Function to compute the Jaccard Index between two sets
double jaccardIndex(const set<int>& set1, const set<int>& set2) {
    // Find intersection and union
    vector<int> intersection, unionSet;
    set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), back_inserter(intersection));
    set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), back_inserter(unionSet));

    // Compute Jaccard Index
    return unionSet.empty() ? 0.0 : static_cast<double>(intersection.size()) / unionSet.size();
}

// Function to compute the maximum sum of Jaccard Indices over all permutations
double maxJaccardSum(const Graph& graph, vector<set<int>> original_partition, ofstream& outfile, string title) {
    double maxSum = 0.0;
    unordered_map<int, set<int>> predicted_labels = graph.getCommunities();

    // Convert map values to a vector of sets
    vector<set<int>> predicted_partition;
    for (const auto& community : predicted_labels) {
        predicted_partition.push_back(community.second);
    }

    // Pad with empty sets for better comparision
    while (predicted_partition.size() < original_partition.size()) {
        predicted_partition.push_back(set<int>());
    }
    while (predicted_partition.size() > original_partition.size()) {
        original_partition.push_back(set<int>());
    }

    vector<pair<set<int>, set<int>>> jaccard_index_pairs;
    while(!original_partition.empty()) {
        double best_pair_score = 0.0;
        size_t best_original_index = 0;
        size_t best_predicted_index = 0;
        // Find the best pair based on Jaccard index
        for (size_t i = 0; i < original_partition.size(); ++i) {
            for (size_t j = 0; j < predicted_partition.size(); ++j) {
                double current_pair_score = jaccardIndex(original_partition[i], predicted_partition[j]);
                if (current_pair_score > best_pair_score) {
                    best_pair_score = current_pair_score;
                    best_original_index = i;
                    best_predicted_index = j;
                }
            }
        }
        // Store the best matching pair
        jaccard_index_pairs.emplace_back(original_partition[best_original_index], predicted_partition[best_predicted_index]);

        // Erase the matched elements from original and predicted partitions
        original_partition.erase(original_partition.begin() + best_original_index);
        predicted_partition.erase(predicted_partition.begin() + best_predicted_index);
        maxSum += best_pair_score;
    }

    outfile << "JaccardSum Difference: Original Partition vs " << title << " Partition" << endl;
    for (const auto& jaccard_pair: jaccard_index_pairs) {
        // Print original_partition and matched predicted_partition set for clarity
        outfile << "{";
        for (auto it = jaccard_pair.first.begin(); it != jaccard_pair.first.end(); ++it) {
            if (it != jaccard_pair.first.begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "} vs {";
        for (auto it = jaccard_pair.second.begin(); it != jaccard_pair.second.end(); ++it) {
            if (it != jaccard_pair.second.begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "}" << endl;
    }
    outfile << endl;

    return maxSum / jaccard_index_pairs.size();
}

double symmetricDifference(const Graph& graph, unordered_map<int, set<int>> original_labels) {
    int result = 0;
    set<int> predicted_label_set, original_label_set;
    unordered_map<int, set<int>> predicted_labels;
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

double nodeOverlapAccuracy(const Graph& graph, vector<set<int>> original_partition, ofstream& outfile, string title) {
    int correct_count = 0;
    unordered_map<int, set<int>> predicted_labels = graph.getCommunities();
    // Convert map values to a vector of sets
    vector<set<int>> predicted_partition;
    for (const auto& community : predicted_labels) {
        predicted_partition.push_back(community.second);
    }

    // Pad with empty sets for better comparision
    while (predicted_partition.size() < original_partition.size()) {
        predicted_partition.push_back(set<int>());
    }
    while (predicted_partition.size() > original_partition.size()) {
        original_partition.push_back(set<int>());
    }

    vector<pair<set<int>, set<int>>> accurate_pairs;
    while(!original_partition.empty()) {
        int max_common_count = 0;
        size_t best_original_index = 0;
        size_t best_predicted_index = 0;

        // Find the best pair based on set difference accuracy
        for (size_t i = 0; i < original_partition.size(); ++i) {
            for (size_t j = 0; j < predicted_partition.size(); ++j) {
                vector<int> intersection;
                set_intersection(
                    original_partition[i].begin(), original_partition[i].end(),
                    predicted_partition[j].begin(), predicted_partition[j].end(),
                    back_inserter(intersection)
                );
                if (intersection.size() > max_common_count) {
                    max_common_count = intersection.size();
                    best_original_index = i;
                    best_predicted_index = j;
                }
            }
        }
        // Store the best matching pair
        accurate_pairs.emplace_back(original_partition[best_original_index], predicted_partition[best_predicted_index]);

        // Erase the matched elements from original and predicted partitions
        original_partition.erase(original_partition.begin() + best_original_index);
        predicted_partition.erase(predicted_partition.begin() + best_predicted_index);
        correct_count += max_common_count;
    }

    int node_movement = 0;
    for (const auto& acc_pair: accurate_pairs) {
        vector<int> difference;
        set_difference(
            acc_pair.first.begin(), acc_pair.first.end(),
            acc_pair.second.begin(), acc_pair.second.end(),
            back_inserter(difference)
        );
        node_movement += difference.size();
    }
    if (title != "") {
        outfile << title << " :: ";
    }
    outfile << "Number of node out of place: " << node_movement << endl;

    return static_cast<double>(correct_count) / graph.nodes.size();
}

double edgeClassificationAccuracy(Graph& predicted_graph, Graph& original_graph) {
    int weighted_correct_count = 0;
    for (const auto& node: predicted_graph.nodes) {
        for (const auto& edge: node->edgeList) {
            int original_src_label = original_graph.getNode(node->id)->label;
            int original_dest_label = original_graph.getNode(edge.first->id)->label;
            if ((original_src_label == original_dest_label && node->label == edge.first->label) || (original_src_label != original_dest_label && node->label != edge.first->label)) {
                weighted_correct_count += edge.second;
            }
        }
    }

    return static_cast<double>(weighted_correct_count) / (2.0 * predicted_graph.getTotalEdges());
}

double maximalMatchingAccuracy(Graph& predicted_graph, Graph& original_graph, ofstream& outfile, string title) {
    vector<vector<int>> cost_matrix;
    vector<set<int>> original_partition, predicted_partition;
    for (const auto& pair: predicted_graph.getCommunities()) {
        predicted_partition.push_back(pair.second);
    }
    for (const auto& pair: original_graph.getCommunities()) {
        original_partition.push_back(pair.second);
    }

    // Calculate cost matrix
    for (const auto& row_comm: original_partition) {
        vector<int> row_cost;
        for (const auto& col_comm: predicted_partition) {
            vector<int> intersection;
            set_intersection(
                row_comm.begin(), row_comm.end(),
                col_comm.begin(), col_comm.end(),
                back_inserter(intersection)
            );
            row_cost.push_back(intersection.size());
        }
        cost_matrix.push_back(row_cost);
    }

    vector<int> assignment;
    HungarianAlgorithm hun(cost_matrix);
    int max_cost = hun.solveAssignmentProblem(assignment);

    outfile << "Maximal Matching: Original Partition vs " << title << " Partition" << endl;
    for (int i = 0; i < original_partition.size(); ++i) {
        outfile << "{";
        for (auto it = original_partition[i].begin(); it != original_partition[i].end(); ++it) {
            if (it != original_partition[i].begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "} vs {";
        for (auto it = predicted_partition[assignment[i]].begin(); it != predicted_partition[assignment[i]].end(); ++it) {
            if (it != predicted_partition[assignment[i]].begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "}" << endl;
    }
    outfile << endl;

    return max_cost / original_graph.nodes.size();
}
