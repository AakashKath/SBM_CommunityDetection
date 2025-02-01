#ifndef QUALITY_MEASURES_H
#define QUALITY_MEASURES_H

#include "src/graph.h"
#include <set>
#include <queue>
#include <limits>
#include <tuple>
#include <sys/resource.h>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

using namespace std;

double modularity(const Graph& graph, int totalEdges = -1);
double symmetricDifference(const Graph& graph, unordered_map<int, set<int>> original_labels);
long getRAMUsage();
double f1Score(const Graph& graph, unordered_map<int, int> original_labels);
// edgeGraph used for original graph as we dont keep edge information on this graph
double loglikelihood(const Graph& graph, const Graph& edgeGraph);
double embeddedness(const Graph& graph);
bool get_cpu_times(size_t &idle_time, size_t &total_time);
vector<size_t> get_cpu_times();
double nodeOverlapAccuracy(const Graph& graph, vector<set<int>> original_partition, ofstream& outfile, string title = "");
double maxJaccardSum(const Graph& graph, vector<set<int>> original_partition, ofstream& outfile, string title = "Predicted");
double edgeClassificationAccuracy(Graph& predicted_graph, Graph& original_graph);

#endif // QUALITY_MEASURES_H
