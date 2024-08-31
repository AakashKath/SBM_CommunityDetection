#include "gtest/gtest.h"
#include "utils/sequence_generator.h"
#include "utils/quality_measures.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include <iomanip>
#include <chrono>

TEST(RunTime, BasicTest) {
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
        static unordered_map<int, unordered_set<int>> original_labels;

        static void SetUpTestSuite() {
            generated_sequence gs = generateSequence();

            for (const auto& node: gs.sbm.sbm_graph.nodes) {
                original_labels[node.label].insert(node.id);
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
unordered_map<int, unordered_set<int>> InitConf::original_labels;

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

TEST_F(InitConf, SymmetricDifferenceTest) {
    unordered_map<string, double> symmetric_difference_ranking;
    symmetric_difference_ranking.emplace("DCD", symmetricDifference(dcd->c_ll, original_labels));
    symmetric_difference_ranking.emplace("StreamBP", symmetricDifference(bp->bp_graph, original_labels));

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
