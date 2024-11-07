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

double newmansModularity(const Graph& graph, bool useSplitPenality, bool useDensity) {
    double modularity = 0.0;
    int totalEdges = 0;
    unordered_map<int, unordered_map<int, int>> communityEdges{};
    unordered_map<int, int> communityCardinality{};

    for (const auto& srcNode: graph.nodes) {
        for (const auto& edge: srcNode->edgeList) {
            communityEdges[srcNode->label][edge.first->label] += edge.second;
            totalEdges += edge.second;
        }
        communityCardinality[srcNode->label]++;
    }
    totalEdges /= 2;

    for (auto& srcCommunityMap: communityEdges) {
        for (auto& destCommunityMap: srcCommunityMap.second) {
            if (srcCommunityMap.first == destCommunityMap.first) {
                destCommunityMap.second /= 2;
            }
        }
    }

    for (const auto& srcCommunityMap: communityEdges) {
        int e_in = 0;
        int e_out = 0;
        double e_ci_cj = 0.0;
        double d_ci = 0.0;
        double d_ci_cj = 0.0;
        for (const auto& destCommunityMap: srcCommunityMap.second) {
            if (srcCommunityMap.first == destCommunityMap.first) {
                e_in += destCommunityMap.second;
            } else {
                e_out += destCommunityMap.second;
            }
            if (useSplitPenality) {
                if (!useDensity) {
                    d_ci_cj = 1.0;
                } else {
                    d_ci_cj = static_cast<double>(destCommunityMap.second) / (communityCardinality.at(srcCommunityMap.first) * communityCardinality.at(destCommunityMap.first));
                }
                if (srcCommunityMap.first != destCommunityMap.first) {
                    e_ci_cj += (destCommunityMap.second * d_ci_cj) / (2.0 * totalEdges);
                }
            }
        }
        int communitySize = communityCardinality.at(srcCommunityMap.first);
        if (!useDensity) {
            d_ci = 1.0;
        } else if (communitySize <= 1) {
            d_ci = 0.0;
        } else {
            d_ci = (2.0 * e_in) / static_cast<double>(communitySize * (communitySize - 1));
        }
        modularity += (static_cast<double>(e_in * d_ci) / totalEdges) - pow((static_cast<double>((2.0 * e_in + e_out) * d_ci) / (2.0 * totalEdges)), 2) - e_ci_cj;
    }

    return modularity;
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
double maxJaccardSum(const Graph& graph, unordered_map<int, set<int>> original_labels, ofstream& outfile) {
    double maxSum = 0.0;
    vector<int> bestPermutation;

    unordered_map<int, set<int>> predicted_labels = graph.getCommunities();

    // Convert map values to a vector of sets
    vector<set<int>> predicted_partition, original_partition;
    for (const auto& community : predicted_labels) {
        predicted_partition.push_back(community.second);
    }
    for (const auto& community : original_labels) {
        original_partition.push_back(community.second);
    }

    // Pad with empty sets for better comparision
    while (predicted_partition.size() < original_partition.size()) {
        predicted_partition.push_back(set<int>());
    }
    while (predicted_partition.size() > original_partition.size()) {
        original_partition.push_back(set<int>());
    }

    vector<int> perm(predicted_partition.size());
    iota(perm.begin(), perm.end(), 0); // Initialize perm with indices 0, 1, ..., n-1

    // Generate all permutations of indices
    do {
        double currentSum = 0.0;

        // Calculate the sum of Jaccard Indices for this permutation
        for (size_t i = 0; i < original_partition.size(); ++i) {
            currentSum += jaccardIndex(original_partition[i], predicted_partition[perm[i]]);
        }

        // Update maxSum and best permutation if the current sum is larger
        if (currentSum > maxSum) {
            maxSum = currentSum;
            bestPermutation = perm;
        }

    } while (next_permutation(perm.begin(), perm.end()));

    outfile << "Best Permutation:" << endl;
    for (size_t i = 0; i < original_partition.size(); ++i) {
        outfile << "original_partition[" << i << "] vs predicted_partition[" << bestPermutation[i] << "] - ";

        // Print original_partition and matched predicted_partition set for clarity
        outfile << "{";
        for (auto it = original_partition[i].begin(); it != original_partition[i].end(); ++it) {
            if (it != original_partition[i].begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "} vs {";
        for (auto it = predicted_partition[bestPermutation[i]].begin(); it != predicted_partition[bestPermutation[i]].end(); ++it) {
            if (it != predicted_partition[bestPermutation[i]].begin()) outfile << ", ";
            outfile << *it;
        }
        outfile << "}" << endl;
    }
    outfile << endl;

    return maxSum / original_partition.size();
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

double accuracy(const Graph& graph, unordered_map<int, int> original_labels, int numberCommunities, ofstream& outfile) {
    double max_accuracy = 0.0;
    vector<int> bestPermutation;
    vector<int> perm(max(static_cast<int>(numberCommunities), static_cast<int>(graph.getCommunities().size())));
    iota(perm.begin(), perm.end(), 0); // Fill with 0, 1, 2, ..., k-1

    // Iterate over all permutations of the label set
    do {
        int correct_count = 0;

        // Loop over all vertices
        for (const auto& node: graph.nodes) {
            int estimated_label = perm[node->label]; // Get the permuted expected label
            int true_label = original_labels.at(node->id); // Get the true label

            if (estimated_label == true_label) {
                correct_count++; // Count correct matches
            }
        }

        // Compute accuracy for this permutation
        double accuracy = static_cast<double>(correct_count) / graph.nodes.size();
        if (accuracy > max_accuracy) {
            max_accuracy = accuracy; // Keep track of the best accuracy
            bestPermutation = perm;
        }

    } while (next_permutation(perm.begin(), perm.end())); // Generate next permutation

    int nodeMovement = 0;
    for (const auto& node: graph.nodes) {
        if (bestPermutation[node->label] != original_labels.at(node->id)) {
            nodeMovement++;
        }
    }
    outfile << "Number of node to be moved: " << nodeMovement << endl;
    outfile << "Ratio of node to be moved w.r.t. total nodes: " << static_cast<double>(nodeMovement) / graph.nodes.size() << endl;

    return max_accuracy;
}
