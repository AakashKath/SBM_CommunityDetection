#include "ip_solver.h"


IPSolver::IPSolver(
    Graph graph,
    int numberCommunities,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    ip_graph(graph),
    numberCommunities(numberCommunities),
    total_edges(graph.getTotalEdges())
{
    // Add edges
    for (const auto& [src_id, dest_id]: addedEdges) {
        ip_graph.addUndirectedEdge(src_id, dest_id);
        total_edges += 1;
    }

    // Solve the ILP
    solveIP();
}

IPSolver::~IPSolver() {
    // Nothing to clean
}

void IPSolver::solveIP(double gap_threshold) {
    if (total_edges == 0) {
        cerr << "SolveILP: No edges in the graph." << endl;
        return;
    }

    // Create the solver
    unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        cerr << "Solver not available." << endl;
        return;
    }

    // Decision variables Xuv
    unordered_map<string, MPVariable*> Xuv;
    for (const auto& node1: ip_graph.nodes) {
        for (const auto& node2: ip_graph.nodes) {
            Xuv[to_string(node1->id) + "_" + to_string(node2->id)] = solver->MakeIntVar(0.0, 1.0, "X" + to_string(node1->id) + "_" + to_string(node2->id));
        }
    }

    int number_of_threads = thread::hardware_concurrency();

    absl::Status status = solver->SetNumThreads(number_of_threads);
    if (status.ok()) {
        cout << "Number of threads set to " << number_of_threads << "." << endl;
    } else {
        cout << "Number of threads not set." << endl;
    }

    // Add constraints
    addReflexivityConstraints(solver.get(), Xuv);
    addSymmetryConstraints(solver.get(), Xuv);
    addTransitivityConstraints(solver.get(), Xuv);
    addCommunityConstraints(solver.get(), Xuv);

    // Objective function
    MPObjective* const objective = addObjective(solver.get(), Xuv);

    // Set gap termination criteria
    solver->SetSolverSpecificParametersAsString("limits/gap=" + to_string(gap_threshold));

    // Solve the ILP
    const MPSolver::ResultStatus result_status = solver->Solve();

    // Mapping node to respective community
    if (result_status == MPSolver::OPTIMAL) {
        cout << "Optimal solution found!" << endl;
        updateNodeLabels(Xuv);
        cout << "Maximum modularity: " << objective->Value() << endl;
    } else {
        cout << "No optimal solution found." << endl;
    }
}

void IPSolver::addReflexivityConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv) {
    // Constraints: Reflexivity
    for (const auto& node: ip_graph.nodes) {
        auto constraint = solver->MakeRowConstraint(1.0, 1.0);
        constraint->SetCoefficient(Xuv[to_string(node->id) + "_" + to_string(node->id)], 1.0);
    }
}

void IPSolver::addSymmetryConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv) {
    // Constraints: Symmetry
    for (int i = 0; i < ip_graph.nodes.size(); ++i) {
        for (int j = i + 1; j < ip_graph.nodes.size(); ++j) {
            auto constraint = solver->MakeRowConstraint(0.0, 0.0);
            constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], 1.0);
            constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[i]->id)], -1.0);
        }
    }
}

void IPSolver::addTransitivityConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv) {
    // Constraints: Transitivity
    for (int i = 0; i < ip_graph.nodes.size(); ++i) {
        for (int j = i + 1; j < ip_graph.nodes.size(); ++j) {
            for (int k = j + 1; k < ip_graph.nodes.size(); ++k) {
                // i -> j, j -> k, i -> k
                auto constraint = solver->MakeRowConstraint(-MPSolver::infinity(), 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[k]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[k]->id)], -2.0);

                // i -> k, i -> j, j -> k
                constraint = solver->MakeRowConstraint(-MPSolver::infinity(), 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[k]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[k]->id)], -2.0);

                // j -> k, i -> k, i -> j
                constraint = solver->MakeRowConstraint(-MPSolver::infinity(), 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[k]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[k]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], -2.0);
            }
        }
    }
}

void IPSolver::addCommunityConstraints(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv) {
    int number_nodes = ip_graph.nodes.size();
    int number_nodes_per_community = number_nodes / numberCommunities;

    // Constraint: Sum of Xuv for a fixed u is equal to the number of nodes in the community
    for (const auto& node1: ip_graph.nodes) {
        auto constraint = solver->MakeRowConstraint(number_nodes_per_community, number_nodes_per_community);
        for (const auto& node2: ip_graph.nodes) {
            constraint->SetCoefficient(Xuv[to_string(node1->id) + "_" + to_string(node2->id)], 1.0);
        }
    }
}

MPObjective* const IPSolver::addObjective(MPSolver* solver, unordered_map<string, MPVariable*>& Xuv) {
    // Objective function
    MPObjective* const objective = solver->MutableObjective();
    for (const auto& node1: ip_graph.nodes) {
        for (const auto& node2: ip_graph.nodes) {
            double edge_weight = 0.0;
            for (const auto& edge: node1->edgeList) {
                if (edge.first->id == node2->id) {
                    edge_weight = edge.second;
                    break;
                }
            }
            double degree_product = static_cast<double>(node1->degree * node2->degree) / (2.0 * total_edges);
            double coeff = (1 / (2.0 * total_edges)) * static_cast<double>(edge_weight - degree_product);
            objective->SetCoefficient(Xuv[to_string(node1->id) + "_" + to_string(node2->id)], coeff);
        }
    }
    objective->SetMaximization();

    return objective;
}

void IPSolver::updateNodeLabels(unordered_map<string, MPVariable*>& Xuv) {
    // Community identifier
    int clusterId = 0;

    // Mapping node to community
    unordered_map<int, int> nodeToCluster{};
    for (const auto& node1: ip_graph.nodes) {
        for (const auto& node2: ip_graph.nodes) {
            if (Xuv[to_string(node1->id) + "_" + to_string(node2->id)]->solution_value() == 1.0) {
                // If node1 is not assigned to any community, assign it to a new community
                if (nodeToCluster.find(node1->id) == nodeToCluster.end()) {
                    nodeToCluster[node1->id] = clusterId++;
                }
                // Assign node2 to the same community as node1
                if (nodeToCluster.find(node2->id) == nodeToCluster.end()) {
                    nodeToCluster[node2->id] = nodeToCluster[node1->id];
                }
            }
        }
    }

    // Assign the community labels to the nodes
    for (const auto& pair : nodeToCluster) {
        Node* node = ip_graph.getNode(pair.first);
        node->label = pair.second;
    }
}
