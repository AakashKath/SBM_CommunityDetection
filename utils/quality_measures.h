#ifndef QUALITY_MEASURES_H
#define QUALITY_MEASURES_H

#include "src/graph.h"
#include <unordered_set>
#include <queue>
#include <limits>
#include <tuple>

using namespace std;

double modularity(const Graph& graph);
double symmetricDifference(const Graph& graph, unordered_map<int, unordered_set<int>> original_labels);

#endif // QUALITY_MEASURES_H
