#include "ip_solver.h"


IPSolver::IPSolver(
    Graph graph,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    ip_graph(graph),
    total_edges(graph.getTotalEdges())
{
    // Add edges
    for (const auto& [src_id, dest_id]: addedEdges) {
        ip_graph.addUndirectedEdge(src_id, dest_id);
    }

    // Solve the ILP
    solve_ilp();
}

IPSolver::~IPSolver() {
    // Nothing to clean
}

void IPSolver::solve_ilp() {
    // Create the solver
    unique_ptr<MPSolver> solver(MPSolver::CreateSolver("CBC"));
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

    // Constraints: Reflexivity
    for (const auto& node: ip_graph.nodes) {
        auto constraint = solver->MakeRowConstraint(1.0, 1.0);
        constraint->SetCoefficient(Xuv[to_string(node->id) + "_" + to_string(node->id)], 1.0);
    }

    // Constraints: Symmetry
    for (int i = 0; i < ip_graph.nodes.size(); ++i) {
        for (int j = i + 1; j < ip_graph.nodes.size(); ++j) {
            auto constraint = solver->MakeRowConstraint(0.0, 0.0);
            constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], 1.0);
            constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[i]->id)], -1.0);
        }
    }

    // Constraints: Transitivity
    for (int i = 0; i < ip_graph.nodes.size(); ++i) {
        for (int j = i + 1; j < ip_graph.nodes.size(); ++j) {
            for (int k = j + 1; k < ip_graph.nodes.size(); ++k) {
                auto constraint = solver->MakeRowConstraint(0.0, 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[j]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[j]->id) + "_" + to_string(ip_graph.nodes[k]->id)], 1.0);
                constraint->SetCoefficient(Xuv[to_string(ip_graph.nodes[i]->id) + "_" + to_string(ip_graph.nodes[k]->id)], -2.0);
            }
        }
    }

    if (total_edges == 0) {
        cerr << "SolveILP: No edges in the graph." << endl;
        return;
    }

    // Define the objective: Maximize the total weight of selected vertices
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

    // Solve the ILP
    const MPSolver::ResultStatus result_status = solver->Solve();

    // Mapping node to cluster
    unordered_map<int, int> nodeToCluster{};
    if (result_status == MPSolver::OPTIMAL) {
        int clusterId = 0;  // Cluster identifier

        cout << "Optimal solution found!" << endl;
        for (const auto& node1: ip_graph.nodes) {
            for (const auto& node2: ip_graph.nodes) {
                if (Xuv[to_string(node1->id) + "_" + to_string(node2->id)]->solution_value() == 1.0) {
                    // If node1 is not assigned to any cluster, assign it to a new cluster
                    if (nodeToCluster.find(node1->id) == nodeToCluster.end()) {
                        nodeToCluster[node1->id] = clusterId++;
                    }
                    // Assign node2 to the same cluster as node1
                    if (nodeToCluster.find(node2->id) == nodeToCluster.end()) {
                        nodeToCluster[node2->id] = nodeToCluster[node1->id];
                    }
                }
            }
        }
        cout << "Maximum modularity: " << objective->Value() << endl;
    } else {
        cout << "No optimal solution found." << endl;
    }

    // Assign the cluster labels to the nodes
    for (const auto& pair : nodeToCluster) {
        Node* node = ip_graph.getNode(pair.first);
        node->label = pair.second;
    }
}
