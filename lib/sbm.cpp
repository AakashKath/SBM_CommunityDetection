#include "sbm.h"
#include <random>
#include <ctime>
#include <algorithm>

Sbm::Sbm(int numberNodes, int numberCommunities): numberNodes(numberNodes), numberCommunities(numberCommunities), sbm_graph(numberNodes) {
    generateProbabilityDistribution();
    generateProbabilityMatrix();
    sbm_graph = generateSbm();
}

Sbm::~Sbm() {
    // Nothing to clean
}

void Sbm::generateProbabilityDistribution() {
    mt19937 rng(static_cast<unsigned int>(time(nullptr)));
    uniform_real_distribution<double> dist(0.0, 0.1);

    p.resize(numberCommunities);
    double sum = 0.0;
    for (int i = 0; i < numberCommunities; ++i) {
        p[i] = dist(rng);
        sum += p[i];
    }

    // Normalize to ensure the sum of probabilities is 1
    for (int i = 0; i < numberCommunities; ++i) {
        p[i] = round((p[i]*100.0)/sum)/100;
    }
}

void Sbm::generateProbabilityMatrix() {
    mt19937 rng(static_cast<unsigned int>(time(nullptr)));
    uniform_real_distribution<double> dist(0.0, 1.0);

    W.resize(numberCommunities, vector<double>(numberCommunities));
    for (int i = 0; i < numberCommunities; ++i) {
        for(int j = 0; j < numberCommunities; ++j) {
            W[i][j] = dist(rng);
            W[j][i] = W[i][j]; // Ensure symmetry
        }
    }
}

Graph Sbm::generateSbm() {
    // Seed the random number generator
    mt19937 rng(static_cast<unsigned int>(time(nullptr)));

    // Create a distribution for the labels based on p
    discrete_distribution<int> labelDistribution(p.begin(), p.end());

    // Create a graph instance
    Graph graph(numberNodes);

    // Assign labels to vertices
    for (int i = 0; i < numberNodes; ++i) {
       graph.nodes[i].label = labelDistribution(rng);
    }

    // Iterate over all pairs of vertices
    for (int u = 0; u < numberNodes; ++u) {
        for (int v = u + 1; v < numberNodes; ++v) {
            // Get the label of u and v
            int labelU = graph.nodes[u].label;
            int labelV = graph.nodes[v].label;
            
            // Generate a random number to decide if an edge exists
            std::uniform_real_distribution<double> probDistribution(0.0, 1.0);
            double randomValue = probDistribution(rng);
            
            // Check if the random value is less than the probability given by W
            if (randomValue < W[labelU][labelV]) {
                graph.nodes[u].edgeList.push_back(v);
                graph.nodes[v].edgeList.push_back(u); // Since the graph is undirected
            }
        }
    }

    return graph;
}

vector<int> Sbm::listLabels() {
    vector<int> labels(sbm_graph.nodes.size());
    for (size_t i = 0; i < sbm_graph.nodes.size(); ++i) {
        labels[i] = sbm_graph.nodes[i].label;
    }

    return labels;
}
