#ifndef SEQUENCE_GENERATOR_H
#define SEQUENCE_GENERATOR_H

#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "src/sbm.h"
#include <filesystem>

using namespace std;

struct generated_sequence {
    Sbm sbm;
    int algorithm_number;
    int radius;
    vector<pair<int, int>> addedEdges;
    vector<pair<int, int>> removedEdges;
    string resultDirectory;
};

generated_sequence generateSequence(string filename = "default.json");

#endif // SEQUENCE_GENERATOR_H
