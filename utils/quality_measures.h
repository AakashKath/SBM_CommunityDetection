#ifndef QUALITY_MEASURES_H
#define QUALITY_MEASURES_H

#include "src/graph.h"
#include <unordered_set>
#include <queue>
#include <limits>
#include <tuple>
#include <sys/resource.h>
#include <fstream>
#include <cmath>

using namespace std;

double modularity(const Graph& graph);
double symmetricDifference(const Graph& graph, unordered_map<int, unordered_set<int>> original_labels);
double getCPUUsage();
long getRAMUsage();
double f1Score(const Graph& graph, unordered_map<int, int> original_labels);
// edgeGraph used for original graph as we dont keep edge information on this graph
double loglikelihood(const Graph& graph, const Graph& edgeGraph);
double embeddedness(const Graph& graph);

#endif // QUALITY_MEASURES_H
