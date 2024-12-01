#include "utilities.h"


Community::Community() {
    // Nothing to add
}

Community::~Community() {
    // Nothing to clean
}

double newmansModularity(unordered_map<int, Community> communities, int total_edges) {
    double modularity = 0.0;
    for (const auto& pair: communities) {
        modularity += modularityContributionByCommunity(pair.second, total_edges);
    }

    return modularity;
}

double newmansModularity(const Graph& graph) {
    double modularity = 0.0;
    int totalEdges = 0;
    unordered_map<int, int> e_in{};
    unordered_map<int, int> e_out{};
    set<int> communities{};

    for (const auto& srcNode: graph.nodes) {
        for (const auto& edge: srcNode->edgeList) {
            if (srcNode->label == edge.first->label) {
                e_in[srcNode->label] += edge.second;
            } else {
                e_out[srcNode->label] += edge.second;
            }
            totalEdges += edge.second;
        }
        communities.emplace(srcNode->label);
    }
    totalEdges /= 2;

    for (auto& mem: e_in) {
        mem.second /= 2;
    }

    for (const auto& comm: communities) {
        try {
            e_in.at(comm);
        } catch (const out_of_range& e) {
            e_in[comm] = 0;
        }
        try {
            e_out.at(comm);
        } catch (const out_of_range& e) {
            e_out[comm] = 0;
        }
    }

    for (const auto& comm: communities) {
        modularity += static_cast<double>(e_in.at(comm)) / totalEdges - pow(static_cast<double>(2.0 * e_in.at(comm) + e_out.at(comm)) / (2.0 * totalEdges), 2);
    }

    return modularity;
}

double modularityContributionByCommunity(const Community& comm, int total_edges) {
    return getModularity(comm.e_in, comm.e_out, total_edges);
}

double getModularity(int e_in, int e_out, int total_edges) {
    return (static_cast<double>(e_in) / total_edges) - pow((static_cast<double>(2.0 * e_in + e_out) / (2.0 * total_edges)), 2);
}

double newmansModularity_(const Graph& graph, bool useSplitPenality, bool useDensity) {
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
