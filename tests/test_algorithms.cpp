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

// CPU and RAM usage by the algorithms. Less the better.
TEST_F(SkipTestOnWindows, MemoryUsageTest) {
    generated_sequence gs = generateSequence();

    vector<tuple<string, double, long>> memory_ranking;

    double dcd_start_cpu = getCPUUsage();
    long dcd_start_ram = getRAMUsage();
    DynamicCommunityDetection dcd(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
    double dcd_end_cpu = getCPUUsage();
    long dcd_end_ram = getRAMUsage();

    memory_ranking.emplace_back("DCD", dcd_end_cpu - dcd_start_cpu, dcd_end_ram - dcd_start_ram);

    double bp_start_cpu = getCPUUsage();
    long bp_start_ram = getRAMUsage();
    BeliefPropagation bp(
        gs.sbm.sbm_graph,
        gs.sbm.numberCommunities,
        gs.radius,
        gs.sbm.intraCommunityEdgeProbability,
        gs.sbm.interCommunityEdgeProbability,
        gs.addedEdges,
        gs.removedEdges
    );
    double bp_end_cpu = getCPUUsage();
    long bp_end_ram = getRAMUsage();

    memory_ranking.emplace_back("StreamBP", bp_end_cpu - bp_start_cpu, bp_end_ram - bp_start_ram);

    EXPECT_GT(memory_ranking.size(), 0);

    // Print the ranking
    int index = 1;
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << setw(25) << "CPU Usage (in milliseconds)" << "Memory Usage (in KB)" << endl;
    for (const auto& rank: memory_ranking) {
        cout << left << setw(6) << index++ << setw(20) << get<0>(rank) << setw(25) << fixed << setprecision(4) << get<1>(rank) << get<2>(rank) << endl;
    }
}

// Run time of the algorithms. Less the better.
TEST(RunTimeTest, BasicTest) {
    generated_sequence gs = generateSequence();

    auto dcd_start = chrono::high_resolution_clock::now();
    DynamicCommunityDetection dcd(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
    auto dcd_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> dcd_duration = dcd_end - dcd_start;

    auto bp_start = chrono::high_resolution_clock::now();
    BeliefPropagation bp(
        gs.sbm.sbm_graph,
        gs.sbm.numberCommunities,
        gs.radius,
        gs.sbm.intraCommunityEdgeProbability,
        gs.sbm.interCommunityEdgeProbability,
        gs.addedEdges,
        gs.removedEdges
    );
    auto bp_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> bp_duration = bp_end - bp_start;

    unordered_map<string, double> runtime_ranking;
    runtime_ranking.emplace("DCD", dcd_duration.count());
    runtime_ranking.emplace("StreamBP", bp_duration.count());

    EXPECT_GT(runtime_ranking.size(), 0);

    // Print the ranking
    int index = 1;
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Runtime (in milliseconds)" << endl;
    for (const auto& rank: runtime_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

class InitConf : public ::testing::Test {
    public:
        static DynamicCommunityDetection* dcd;
        static BeliefPropagation* bp;
        static unordered_map<int, unordered_set<int>> community_to_node_mapping;
        static unordered_map<int, int> node_community_to_mapping;
        static double interCommunityEdgeProbability;
        static double intraCommunityEdgeProbability;
        static Graph original_graph;

        static void SetUpTestSuite() {
            generated_sequence gs = generateSequence();

            intraCommunityEdgeProbability = gs.sbm.intraCommunityEdgeProbability;
            interCommunityEdgeProbability = gs.sbm.interCommunityEdgeProbability;
            original_graph = gs.sbm.sbm_graph;

            for (const auto& node: gs.sbm.sbm_graph.nodes) {
                community_to_node_mapping[node.label].insert(node.id);
                node_community_to_mapping.emplace(node.id, node.label);
            }

            // Run all the algorithms
            dcd = new DynamicCommunityDetection(gs.sbm.sbm_graph, gs.sbm.numberCommunities, gs.addedEdges, gs.removedEdges);
            bp = new BeliefPropagation(
                gs.sbm.sbm_graph,
                gs.sbm.numberCommunities,
                gs.radius,
                gs.sbm.intraCommunityEdgeProbability,
                gs.sbm.interCommunityEdgeProbability,
                gs.addedEdges,
                gs.removedEdges
            );
        }

        static void TearDownTestSuite() {
            delete dcd;
            delete bp;
        }
};

DynamicCommunityDetection* InitConf::dcd = nullptr;
BeliefPropagation* InitConf::bp = nullptr;
unordered_map<int, unordered_set<int>> InitConf::community_to_node_mapping;
unordered_map<int, int> InitConf::node_community_to_mapping;
double InitConf::interCommunityEdgeProbability;
double InitConf::intraCommunityEdgeProbability;
Graph InitConf::original_graph = Graph(0);

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
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Modularity Value" << endl;
    for (const auto& rank: modularity_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
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
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Symmetric Difference Value" << endl;
    for (const auto& rank: symmetric_difference_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// Harmonic mean of precision and recall. Higher values indicate better performance.
TEST_F(InitConf, F1ScoreTest) {
    unordered_map<string, double> f1_score_ranking;
    f1_score_ranking.emplace("DCD", f1Score(dcd->c_ll, node_community_to_mapping));
    f1_score_ranking.emplace("StreamBP", f1Score(bp->bp_graph, node_community_to_mapping));

    EXPECT_GT(f1_score_ranking.size(), 0);
    for (const auto& rank: f1_score_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "F1 Score" << endl;
    for (const auto& rank: f1_score_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}

// How close is the predicted graph to the original graph. Higher log-likelihood distance indicates more closer to the original graph.
TEST_F(InitConf, LogLikelihoodTest) {
    unordered_map<string, double> log_likelihood_ranking;
    double original_ll = loglikelihood(original_graph, interCommunityEdgeProbability, intraCommunityEdgeProbability);
    double dcd_ll = loglikelihood(dcd->c_ll, interCommunityEdgeProbability, intraCommunityEdgeProbability);
    double bp_ll = loglikelihood(bp->bp_graph, interCommunityEdgeProbability, intraCommunityEdgeProbability);
    log_likelihood_ranking.emplace("DCD", fabs((original_ll - dcd_ll) / original_ll));
    log_likelihood_ranking.emplace("StreamBP", fabs((original_ll - bp_ll) / original_ll));

    EXPECT_GT(log_likelihood_ranking.size(), 0);
    for (const auto& rank: log_likelihood_ranking) {
        EXPECT_TRUE((rank.second >= 0.0) && (rank.second <= 1.0));
    }

    // Print the ranking
    int index = 1;
    cout << left << setw(6) << "Rank" << setw(20) << "Algorithm Name" << "Log-likelihood Distance" << endl;
    for (const auto& rank: log_likelihood_ranking) {
        cout << left << setw(6) << index++ << setw(20) << rank.first << setprecision(4) << rank.second << endl;
    }
}
