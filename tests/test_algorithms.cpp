#include "gtest/gtest.h"
#include "utils/sequence_generator.h"
#include "utils/quality_measures.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include <iomanip>
#include <chrono>

class SkipTestOnWindows : public ::testing::Test {
    protected:
        void SetUp() override {
            #ifdef _WIN32
                GTEST_SKIP() << "Test skipped on Windows. Need to implement a windows usage method.";
            #endif
        }
};

class InitConf : public ::testing::Test {
    public:
        static ofstream outfile;
        static DynamicCommunityDetection* dcd;
        static BeliefPropagation* bp;
        static unordered_map<int, set<int>> community_to_node_mapping;
        static unordered_map<int, int> node_to_community_mapping;
        static double interCommunityEdgeProbability;
        static double intraCommunityEdgeProbability;
        static Graph original_graph;
        static vector<set<int>> community_partition;

        static void SetUpTestSuite() {
            vector<tuple<string, double, long>> memory_ranking;
            size_t start_idle_time = 0;
            size_t start_total_time = 0;
            size_t end_idle_time = 0;
            size_t end_total_time = 0;
            generated_sequence gs = generateSequence();

            intraCommunityEdgeProbability = gs.sbm.intraCommunityEdgeProbability;
            interCommunityEdgeProbability = gs.sbm.interCommunityEdgeProbability;
            original_graph = gs.sbm.sbm_graph;

            node_to_community_mapping = gs.sbm.sbm_graph.getLabels();
            community_to_node_mapping = gs.sbm.sbm_graph.getCommunities();
            // Create community partition
            for (const auto& community : community_to_node_mapping) {
                community_partition.push_back(community.second);
            }

            // Run all the algorithms
            if (!get_cpu_times(start_idle_time, start_total_time)) {
                cerr << "Failed to get CPU times at start." << endl;
            }
            long dcd_start_ram = getRAMUsage();
            auto dcd_start = chrono::high_resolution_clock::now();
            dcd = new DynamicCommunityDetection(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
            auto dcd_end = chrono::high_resolution_clock::now();
            long dcd_end_ram = getRAMUsage();
            if (!get_cpu_times(end_idle_time, end_total_time)) {
                cerr << "Failed to get CPU times at end." << endl;
            }
            double dcd_cpu_usage = 100.0 * (1.0 - (end_idle_time - start_idle_time) / (end_total_time - start_total_time));
            memory_ranking.emplace_back("DCD", dcd_cpu_usage, dcd_end_ram - dcd_start_ram);
            chrono::duration<double, milli> dcd_duration = dcd_end - dcd_start;

            if (!get_cpu_times(start_idle_time, start_total_time)) {
                cerr << "Failed to get CPU times at start." << endl;
            }
            long bp_start_ram = getRAMUsage();
            auto bp_start = chrono::high_resolution_clock::now();
            bp = new BeliefPropagation(
                gs.sbm.sbm_graph,
                gs.sbm.numberCommunities,
                gs.radius,
                gs.sbm.intraCommunityEdgeProbability,
                gs.sbm.interCommunityEdgeProbability,
                gs.addedEdges,
                gs.removedEdges
            );
            auto bp_end = chrono::high_resolution_clock::now();
            long bp_end_ram = getRAMUsage();
            if (!get_cpu_times(end_idle_time, end_total_time)) {
                cerr << "Failed to get CPU times at end." << endl;
            }
            double bp_cpu_usage = 100.0 * (1.0 - (end_idle_time - start_idle_time) / (end_total_time - start_total_time));
            memory_ranking.emplace_back("StreamBP", bp_cpu_usage, bp_end_ram - bp_start_ram);
            chrono::duration<double, milli> bp_duration = bp_end - bp_start;

            // Draw predicted graphs
            bp->bp_graph.draw(TEST_OUTPUT_DIRECTORY + gs.resultDirectory + string("/bp_graph.png"));
            dcd->c_ll.draw(TEST_OUTPUT_DIRECTORY + gs.resultDirectory + string("/dcd.png"));

            outfile.open(TEST_OUTPUT_DIRECTORY + gs.resultDirectory + string("/results.txt"));

            // Print graph details
            outfile << "Number of Nodes: " << gs.sbm.sbm_graph.nodes.size() << endl
                    << "Number of edges: " << gs.addedEdges.size() << endl
                    << "Number of communities: " << gs.sbm.numberCommunities << endl
                    << "Radius: " << gs.radius << endl
                    << "Inter-community edge probability: " << gs.sbm.interCommunityEdgeProbability << endl
                    << "Intra-community edge probability: " << gs.sbm.intraCommunityEdgeProbability << endl;

            // Print Original and expected communities
            outfile << endl << "Original Communities:" << endl;
            for (const auto& community_cluster: community_to_node_mapping) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << "StreamBP Communities:" << endl;
            for (const auto& community_cluster: bp->bp_graph.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }
            outfile << "DCD Communities:" << endl;
            for (const auto& community_cluster: dcd->c_ll.getCommunities()) {
                for (const Node& node: community_cluster.second) {
                    outfile << node.id << " ";
                }
                outfile << endl;
            }

            // Print edges
            outfile << endl << "Edge details:" << endl;
            outfile << left << setw(10) << "Src" << " : " << setw(10) << "Dest" << endl;
            for (const auto& [src, dest] : gs.addedEdges) {
                outfile << left << setw(10) << src << " : " << setw(10) << dest << endl;
            }

            // Print the ranking
            int index = 1;
            outfile << left << setw(6) << "Rank"
                    << setw(20) << "Algorithm Name"
                    << setw(30) << "CPU Usage (in milliseconds)"
                    << setw(20) << "Memory Usage (in KB)" << endl;
            for (const auto& rank: memory_ranking) {
                outfile << left << setw(6) << index++
                        << setw(20) << get<0>(rank)
                        << setw(30) << fixed << setprecision(4) << get<1>(rank)
                        << setw(20) << get<2>(rank) << endl;
            }

            unordered_map<string, double> runtime_ranking;
            runtime_ranking.emplace("DCD", dcd_duration.count());
            runtime_ranking.emplace("StreamBP", bp_duration.count());

            // Print the ranking
            index = 1;
            outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Runtime (in milliseconds)" << endl;
            for (const auto& rank: runtime_ranking) {
                outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
            }
            outfile << endl;
        }

        static void TearDownTestSuite() {
            delete dcd;
            delete bp;

            // Close the file after all tests have completed
            if (outfile.is_open()) {
                outfile.close();
            }
        }
};

ofstream InitConf::outfile;
DynamicCommunityDetection* InitConf::dcd = nullptr;
BeliefPropagation* InitConf::bp = nullptr;
unordered_map<int, set<int>> InitConf::community_to_node_mapping;
unordered_map<int, int> InitConf::node_to_community_mapping;
double InitConf::interCommunityEdgeProbability;
double InitConf::intraCommunityEdgeProbability;
Graph InitConf::original_graph = Graph(0);
vector<set<int>> InitConf::community_partition;

// Measures the strength of the division of a network into communities by comparing the density of edges inside
// communities with the density expected if edges were distributed randomly.
// Higher modularity values indicate better-defined community structures.
TEST_F(InitConf, ModularityTest) {
    unordered_map<string, double> modularity_ranking;
    modularity_ranking.emplace("DCD", modularity(dcd->c_ll));
    modularity_ranking.emplace("StreamBP", modularity(bp->bp_graph));

    EXPECT_GT(modularity_ranking.size(), 0);
    for (const auto& rank: modularity_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Modularity Value" << endl;
    for (const auto& rank: modularity_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Symmetric difference between original and predicted communities. Less the better.
TEST_F(InitConf, SymmetricDifferenceTest) {
    unordered_map<string, double> symmetric_difference_ranking;
    symmetric_difference_ranking.emplace("DCD", symmetricDifference(dcd->c_ll, community_to_node_mapping));
    symmetric_difference_ranking.emplace("StreamBP", symmetricDifference(bp->bp_graph, community_to_node_mapping));

    EXPECT_GT(symmetric_difference_ranking.size(), 0);
    for (const auto& rank: symmetric_difference_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Symmetric Difference Value" << endl;
    for (const auto& rank: symmetric_difference_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Harmonic mean of precision and recall. Higher values indicate better performance.
TEST_F(InitConf, F1ScoreTest) {
    unordered_map<string, double> f1_score_ranking;
    f1_score_ranking.emplace("DCD", f1Score(dcd->c_ll, node_to_community_mapping));
    f1_score_ranking.emplace("StreamBP", f1Score(bp->bp_graph, node_to_community_mapping));

    EXPECT_GT(f1_score_ranking.size(), 0);
    for (const auto& rank: f1_score_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "F1 Score" << endl;
    for (const auto& rank: f1_score_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// How close is the predicted graph to the original graph. Lower log-likelihood distance indicates more closer to the original graph.
TEST_F(InitConf, LogLikelihoodTest) {
    unordered_map<string, double> log_likelihood_ranking;
    // Using bp_graph for original log likelihood as it doesn't make changes to the edge information
    double original_ll = loglikelihood(original_graph, bp->bp_graph);
    double dcd_ll = loglikelihood(dcd->c_ll, dcd->c_ll);
    double bp_ll = loglikelihood(bp->bp_graph, bp->bp_graph);
    log_likelihood_ranking.emplace("DCD", fabs((original_ll - dcd_ll) / original_ll));
    log_likelihood_ranking.emplace("StreamBP", fabs((original_ll - bp_ll) / original_ll));

    EXPECT_GT(log_likelihood_ranking.size(), 0);
    for (const auto& rank: log_likelihood_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Log-likelihood Distance" << endl;
    for (const auto& rank: log_likelihood_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// #edges_within_community / #total_edges. Higher embeddedness means nodes are well-integrated.
// TODO: Can be compared with the original graph
TEST_F(InitConf, EmbeddednessTest) {
    unordered_map<string, double> embeddedness_ranking;
    double original_e = embeddedness(original_graph);
    embeddedness_ranking.emplace("DCD", embeddedness(dcd->c_ll));
    embeddedness_ranking.emplace("StreamBP", embeddedness(bp->bp_graph));

    EXPECT_GT(embeddedness_ranking.size(), 0);

    // Print the ranking
    int index = 1;
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Embeddedness Distance" << endl;
    for (const auto& rank: embeddedness_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Also called MoveTest, determines minimum number of nodes that needs to moved to different community to get same partition
TEST_F(InitConf, NodeOverlapAccuracyTest) {
    unordered_map<string, double> node_overlap_accuracy_ranking;
    node_overlap_accuracy_ranking.emplace("DCD", nodeOverlapAccuracy(dcd->c_ll, community_partition, outfile));
    node_overlap_accuracy_ranking.emplace("StreamBP", nodeOverlapAccuracy(bp->bp_graph, community_partition, outfile));

    EXPECT_GT(node_overlap_accuracy_ranking.size(), 0);
    for (const auto& rank: node_overlap_accuracy_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Node Overlap Accuracy" << endl;
    for (const auto& rank: node_overlap_accuracy_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Jaccard sum compares the best permutation of original and predicted parition
// It compares the ratio of intersection and union
TEST_F(InitConf, JaccardSumTest) {
    unordered_map<string, double> jaccard_sum;
    jaccard_sum.emplace("DCD", maxJaccardSum(dcd->c_ll, community_partition, outfile));
    jaccard_sum.emplace("StreamBP", maxJaccardSum(bp->bp_graph, community_partition, outfile));

    EXPECT_GT(jaccard_sum.size(), 0);
    for (const auto& rank: jaccard_sum) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Max Jaccard Sum" << endl;
    for (const auto& rank: jaccard_sum) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Get ratio of correctly classified (intra or inter community) edges to total edges
TEST_F(InitConf, EdgeClassificationAccuracyTest) {
    unordered_map<string, double> edge_classification_accuracy_ranking;
    edge_classification_accuracy_ranking.emplace("DCD", edgeClassificationAccuracy(dcd->c_ll, original_graph));
    edge_classification_accuracy_ranking.emplace("StreamBP", edgeClassificationAccuracy(bp->bp_graph, original_graph));

    EXPECT_GT(edge_classification_accuracy_ranking.size(), 0);
    for (const auto& rank: edge_classification_accuracy_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Edge Classification Accuracy" << endl;
    for (const auto& rank: edge_classification_accuracy_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Get ratio of correctly classified (intra or inter community) edges to total edges
TEST_F(InitConf, MaximalMatchingAccuracyTest) {
    unordered_map<string, double> maximal_matching_accuracy_ranking;
    maximal_matching_accuracy_ranking.emplace("DCD", maximalMatchingAccuracy(dcd->c_ll, original_graph, outfile));
    maximal_matching_accuracy_ranking.emplace("StreamBP", maximalMatchingAccuracy(bp->bp_graph, original_graph, outfile));

    EXPECT_GT(maximal_matching_accuracy_ranking.size(), 0);
    for (const auto& rank: maximal_matching_accuracy_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    outfile << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Maximal Matching Accuracy" << endl;
    for (const auto& rank: maximal_matching_accuracy_ranking) {
        outfile << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}
