#ifndef TEST_SCRIPT_H
#define TEST_SCRIPT_H

#include <iostream>
#include "src/sbm.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include "approximate_community_detection.h"
#include "ip_solver.h"
#include <filesystem>
#include <fstream>

using namespace std;

void run_test_script(bool draw_graphs = false);
vector<string> splitString(const string& str, char delimiter);

#endif // TEST_SCRIPT_H
