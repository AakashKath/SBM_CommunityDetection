#ifndef SELF_RUN_SCRIPT_H
#define SELF_RUN_SCRIPT_H

#include <iostream>
#include "src/sbm.h"
#include "approximate_community_detection.h"
#include <fstream>
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

using namespace std;

void run_against_self();
void plot_results(string file_prefix, string path_prefix, string condition);

#endif // SELF_RUN_SCRIPT_H
