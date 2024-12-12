#include <iostream>
#include "utils/sequence_generator.h"
#include "dynamic_community_detection.h"
#include "belief_propagation.h"
#include "approximate_community_detection.h"
#include "ortools/linear_solver/linear_solver.h"

using namespace std;
using namespace operations_research;


// Function to display the help message
void displayHelp() {
    cout << "Usage: ./main [options]\n"
        << "Options:\n"
        << "  -f, --filename [string]  Specify the filename"
        << "  -h, --help            Display this help message\n";
}

int main(int argc, char* argv[]) {
    string filename = "default.json";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--filename") == 0) {
            if (i+1 < argc) {
                filename = argv[i + 1];
                ++i; // Skip the next argument as it's the value for nodes
            } else {
                cerr << "Error: --filename or -f requires a string as an argument.\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            displayHelp();
            return 0;
        } else {
            cerr << "Unknown option: " << argv[i] << "\n";
            displayHelp();
            return 1;
        }
    }

    // Example graph as an adjacency list
    // Node format: {node_id: (weight, [neighbors])}
    std::unordered_map<int, std::pair<int, std::vector<int>>> graph = {
        {0, {5, {1, 2}}},
        {1, {3, {0, 2}}},
        {2, {4, {0, 1, 3}}},
        {3, {2, {2}}}
    };

    // Create the solver
    MPSolver solver("Graph_ILP", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);

    // Create variables dynamically
    std::unordered_map<int, MPVariable*> variables;
    for (const auto& [node, data] : graph) {
        variables[node] = solver.MakeIntVar(0.0, 1.0, "x" + std::to_string(node));
    }

    // Add constraints: No two adjacent vertices can be selected simultaneously
    for (const auto& [node, data] : graph) {
        for (int neighbor : data.second) {
            MPConstraint* const constraint = solver.MakeRowConstraint(0.0, 1.0);
            constraint->SetCoefficient(variables[node], 1);
            constraint->SetCoefficient(variables[neighbor], 1);
        }
    }

    // Define the objective: Maximize the total weight of selected vertices
    MPObjective* const objective = solver.MutableObjective();
    for (const auto& [node, data] : graph) {
        objective->SetCoefficient(variables[node], data.first);
    }
    objective->SetMaximization();

    // Solve the problem
    const MPSolver::ResultStatus result_status = solver.Solve();

    // Output the results
    if (result_status == MPSolver::OPTIMAL) {
        std::cout << "Optimal solution found!" << std::endl;
        for (const auto& [node, var] : variables) {
            std::cout << "Node " << node << ": " << var->solution_value() << std::endl;
        }
        std::cout << "Maximum weight: " << objective->Value() << std::endl;
    } else {
        std::cout << "No optimal solution found." << std::endl;
    }

    return 0;
}
