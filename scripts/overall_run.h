#ifndef TEST_SCRIPT_H
#define TEST_SCRIPT_H

#include <iostream>
#include "src/sbm.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include "approximate_community_detection.h"
#include "ip_solver.h"
#include <fstream>
#include "utils/utilities.h"

using namespace std;

void run_all_algorithms(bool draw_graphs = false);

#endif // TEST_SCRIPT_H
