#include "sbm.h"
#include <ctime>
#include <algorithm>
#include <unordered_set>

Sbm::Sbm(int numberNodes, int numberCommunities, double intraCommunityEdgeProbability, double interCommunityEdgeProbability):
    numberNodes(numberNodes), numberCommunities(numberCommunities), intraCommunityEdgeProbability(intraCommunityEdgeProbability),
    interCommunityEdgeProbability(interCommunityEdgeProbability), sbm_graph(numberNodes),
    communityTracker(numberCommunities, vector<int>(numberNodes/numberCommunities, -1)) {

    // Create a random number generator
    random_device rd;
    mt19937 gen(rd());

    // Generate graph with no edges
    sbm_graph = generateSbm();

    // Calculate communityBoundaryThreshold for edge generation
    double chooseNodesFromSameCommunity = numberCommunities * (1.0 / ((numberNodes / numberCommunities + 1) * betal(numberNodes / numberCommunities - 1, 3)));
    double intraCommunityWeight = chooseNodesFromSameCommunity * intraCommunityEdgeProbability;
    double interCommunityWeight = (1.0 / ((numberNodes + 1) * betal(numberNodes - 1, 3)) - chooseNodesFromSameCommunity) * interCommunityEdgeProbability;
    communityBoundaryThreshold = intraCommunityWeight / (interCommunityWeight + intraCommunityWeight);
}

Sbm::~Sbm() {
    // Nothing to clean
}

pair<int, int> Sbm::generateEdge() {
    if (isIntraCommunityEdge()) {
        return generateIntraCommunityEdge();
    }
    return generateInterCommunityEdge();
}

pair<int, int> Sbm::generateInterCommunityEdge() {
    // Create a uniform distribution between communities
    uniform_int_distribution<int> communityDistribution(0, numberCommunities - 1);
    int community1 = communityDistribution(gen);
    int community2;

    do {
        community2 = communityDistribution(gen);
    } while(community1 == community2);

    // Create a uniform distribution between node blocks
    uniform_int_distribution<int> nodeDistribution(0, numberNodes / numberCommunities - 1);
    int offset1 = nodeDistribution(gen);
    int offset2 = nodeDistribution(gen);

    // Return node pair
    return make_pair(communityTracker[community1][offset1], communityTracker[community2][offset2]);
}

pair<int, int> Sbm::generateIntraCommunityEdge() {
    // Create a uniform distribution between communities
    uniform_int_distribution<int> communityDistribution(0, numberCommunities - 1);
    int community = communityDistribution(gen);

    // Create a uniform distribution between node blocks
    uniform_int_distribution<int> nodeDistribution(0, numberNodes / numberCommunities - 1);
    int offset1 = nodeDistribution(gen);
    int offset2;
    do {
        offset2 = nodeDistribution(gen);
    } while(offset1 == offset2);

    return make_pair(communityTracker[community][offset1], communityTracker[community][offset2]);
}

bool Sbm::isIntraCommunityEdge() {
    // Create a uniform distribution in the range [0.0, 1.0)
    uniform_real_distribution<double> dis(0.0, 1.0);

    // Return true for intra community edge, and false for inter community edge
    return dis(gen) < communityBoundaryThreshold;
}

Graph Sbm::generateSbm() {
    // Create a graph instance
    Graph graph(numberNodes);

    uniform_int_distribution<int> dist(0, numberNodes - 1);

    unordered_set<int> usedNumbers;

    int blockSize = numberNodes/numberCommunities;

    for (int i = 0; i < numberCommunities; ++i) {
        for (int j = 0; j < blockSize; ++j) {
            int num;
            do {
                num = dist(gen);
            } while(usedNumbers.find(num) != usedNumbers.end());

            // Update usedNumbers
            usedNumbers.insert(num);

            // Assign labels to vertices
            Node& node = graph.getNode(num);
            node.label = i;
            node.offset = j;

            // Fill tracker matrix
            communityTracker[i][j] = num;
        }
    }

    return graph;
}
