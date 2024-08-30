#include "gtest/gtest.h"
#include "utils/sequence_generator.h"
#include "utils/quality_measures.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include <iomanip>

class InitConf : public ::testing::Test {
    public:
        static DynamicCommunityDetection* dcd;
        static BeliefPropagation* bp;

        static void SetUpTestSuite() {
            generated_sequence gs = generateSequence();

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

TEST_F(InitConf, ModularityCheck) {
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
        cout << left << setw(6) << index << setw(20) << rank.first << rank.second << endl;
    }
}
