#ifndef IP_SOLVER_H
#define IP_SOLVER_H

#include "src/graph.h"
#include "ortools/linear_solver/linear_solver.h"

using namespace std;
using namespace operations_research;


// IPSolver class
class IPSolver {
    public:
        Graph ip_graph;

        IPSolver(Graph graph, vector<pair<int, int>> addedEdges, vector<pair<int, int>> removedEdges);
        ~IPSolver();

    private:
        int total_edges;

        void solveIP();
        void addReflexivityConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv);
        void addSymmetryConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv);
        void addTransitivityConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv);
        MPObjective* const addObjective(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv);
        void updateNodeLabels(unordered_map<string, MPVariable*>& Xuv);
};

#endif // IP_SOLVER_H
