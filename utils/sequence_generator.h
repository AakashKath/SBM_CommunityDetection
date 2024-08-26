#ifndef SEQUENCE_GENERATOR_H
#define SEQUENCE_GENERATOR_H

#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "src/sbm.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"

using namespace std;

void generateSequence(const string& filename = "default.json");

#endif // SEQUENCE_GENERATOR_H
