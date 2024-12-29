#include "approximate_community_detection.h"


ApproximateCommunityDetection::ApproximateCommunityDetection(
    Graph graph,
    int communityCount,
    vector<pair<int, int>> addedEdges,
    vector<pair<int, int>> removedEdges
):
    acd_graph(graph),
    communityCount(communityCount),
    totalEdges(graph.getTotalEdges()),
    gen(random_device{}())
{
    // Assign nodes to random community
    initializePartition();
    createCommunities();

    // Add edges and update communities
    // TODO: Time to implement node removal functionality
    for (const auto& [src_id, dest_id]: addedEdges) {
        auto [src_community, dest_community] = addEdge(src_id, dest_id);

        // Skip if both nodes are in the same community
        if (src_community.id == dest_community.id) {
            continue;
        }

        // Create heapAndMap
        createHeapAndMap(src_community, dest_community);
        createHeapAndMap(dest_community, src_community);

        run2FMAlgorithm(src_community, dest_community);
    }

    // // FM algorithm for all the communities
    // bool no_node_moved = false;
    // while(!no_node_moved) {
    //     no_node_moved = runKFMAlgorithm();
    //     if (!no_node_moved) {
    //         cout << "Made some change." << endl;
    //     }
    // }
}

ApproximateCommunityDetection::~ApproximateCommunityDetection() {
    // Nothing to clean
}

void ApproximateCommunityDetection::initializePartition() {
    int number_nodes = acd_graph.nodes.size();
    uniform_int_distribution<int> dist(0, number_nodes - 1);
    unordered_set<int> usedNumbers;
    int blockSize = number_nodes / communityCount;

    for (int i = 0; i < communityCount; ++i) {
        for (int j = 0; j < blockSize; ++j) {
            int num;
            do {
                num = dist(gen);
            } while (usedNumbers.find(num) != usedNumbers.end());

            // Update usedNumbers
            usedNumbers.insert(num);

            // Assign labels to vertices
            Node* node = acd_graph.getNode(num);
            node->label = i;
            node->offset = j;
        }
    }
}

void ApproximateCommunityDetection::createCommunities() {
    // Push node addresses into their respective communities
    for (const auto& node: acd_graph.nodes) {
        // Fetch or create relevant community
        Community& comm = [&]() -> Community& {
            try {
                return communities.at(node->label);
            } catch (const out_of_range&) {
                Community new_comm;
                new_comm.id = node->label;
                return communities.emplace(node->label, move(new_comm)).first->second;
            }
        }();

        comm.nodes.insert(node.get());
        for (const auto& edge: node.get()->edgeList) {
            if (edge.first->label == node->label) {
                comm.e_in += edge.second;
            } else {
                comm.e_out += edge.second;
            }
        }
    }

    for (auto& comm: communities) {
        comm.second.e_in /= 2;
    }
}

pair<Community&, Community&> ApproximateCommunityDetection::addEdge(int srcId, int destId) {
    Node* src = acd_graph.getNode(srcId);
    Node* dest = acd_graph.getNode(destId);

    acd_graph.addUndirectedEdge(src, dest);
    totalEdges += 1;

    // Update e_in, e_out
    if (src->label == dest->label) {
        Community& comm = communities.at(src->label);
        comm.e_in += 1;
        return make_pair(ref(comm), ref(comm));
    } else {
        Community& src_comm = communities.at(src->label);
        src_comm.e_out += 1;
        Community& dest_comm = communities.at(dest->label);
        dest_comm.e_out += 1;
        return make_pair(ref(src_comm), ref(dest_comm));
    }
}

void ApproximateCommunityDetection::createHeapAndMap(Community& current_comm, Community& involved_comm) {
    // Calculate current and involved community degree
    double current_comm_degree = 0.0;
    for (const auto& node: current_comm.nodes) {
        current_comm_degree += node->degree;
    }
    double involved_comm_degree = 0.0;
    for (const auto& node: involved_comm.nodes) {
        involved_comm_degree += node->degree;
    }

    // Iterate through community nodes to fill heapAndMap
    vector<pair<string, double>> modularity_gains;
    for (const auto& node: current_comm.nodes) {
        double gain = 0.0;

        // Add involved community edges and remove current community edges
        for (const auto& edge: node->edgeList) {
            Node* neighbor = edge.first;
            double edge_weight = static_cast<double>(edge.second);
            if (neighbor->label == current_comm.id) {
                gain -= edge_weight;
            }
            if (neighbor->label == involved_comm.id) {
                gain += edge_weight;
            }
        }

        // Add current community squared degree and remove involved community squared degree
        gain += static_cast<double>(node->degree * (current_comm_degree - node->degree)) / (2.0 * totalEdges);
        gain -= static_cast<double>(node->degree * involved_comm_degree) / (2.0 * totalEdges);

        modularity_gains.emplace_back(to_string(node->id), gain);
    }

    // Fill heapAndMap
    current_comm.node_removal_priority_queue.populateHeapAndMap(modularity_gains);
}

void ApproximateCommunityDetection::run2FMAlgorithm(Community& comm1, Community& comm2) {
    unordered_set<Node*> best_comm1_nodes;
    unordered_set<Node*> best_comm2_nodes;
    unordered_set<int> frozen_node_ids;
    double best_modularity = -1.0;
    while (!comm1.node_removal_priority_queue.isEmpty() || !comm2.node_removal_priority_queue.isEmpty()) {
        // Determine which community will move the node
        Community* main_community = nullptr;
        Community* other_community = nullptr;
        if (comm1.nodes.size() != comm2.nodes.size()) {
            main_community = (comm1.nodes.size() > comm2.nodes.size()) ? &comm1 : &comm2;
            other_community = (main_community == &comm1) ? &comm2 : &comm1;
        } else {
            double comm1_value = comm1.node_removal_priority_queue.getMaxValue();
            double comm2_value = comm2.node_removal_priority_queue.getMaxValue();
            main_community = (comm1_value >= comm2_value) ? &comm1 : &comm2;
            other_community = (main_community == &comm1) ? &comm2 : &comm1;
        }

        // Get top elements from heapAndMap
        Node* node_moved = acd_graph.getNode(stoi(main_community->node_removal_priority_queue.getMaxElementId()));

        // Remove node from main community
        main_community->node_removal_priority_queue.popElement();
        main_community->nodes.erase(node_moved);
        other_community->nodes.insert(node_moved);
        node_moved->label = other_community->id;

        // Update frozen node ids
        frozen_node_ids.insert(node_moved->id);

        // Update relevant information
        for (auto& edge: node_moved->edgeList) {
            Node* neighbor = edge.first;
            int edge_weight = edge.second;

            // Find if neighbor is a frozen node
            bool is_frozen_node = (frozen_node_ids.find(neighbor->id) != frozen_node_ids.end()) ? true: false;

            if (neighbor->label == main_community->id) {
                if (!is_frozen_node) {
                    // Update node removal priority queue
                    double old_value = main_community->node_removal_priority_queue.getValue(to_string(neighbor->id));
                    main_community->node_removal_priority_queue.deleteElement(to_string(neighbor->id));
                    main_community->node_removal_priority_queue.insertElement(to_string(neighbor->id), old_value + 2.0 * edge_weight);
                }

                // Update e_in, e_out for main community
                main_community->e_in -= edge_weight;
                main_community->e_out += edge_weight;
            } else {
                main_community->e_out -= edge_weight;
            }
            if (neighbor->label == other_community->id) {
                if (!is_frozen_node) {
                    // Update node removal priority queue
                    double old_value = other_community->node_removal_priority_queue.getValue(to_string(neighbor->id));
                    other_community->node_removal_priority_queue.deleteElement(to_string(neighbor->id));
                    other_community->node_removal_priority_queue.insertElement(to_string(neighbor->id), old_value - 2.0 * edge_weight);
                }

                // Update e_in, e_out for other community
                other_community->e_in += edge_weight;
                other_community->e_out -= edge_weight;
            } else {
                other_community->e_out += edge_weight;
            }
        }

        if (main_community->nodes.size() == other_community->nodes.size()) {
            // Calculate modularity for both commuitites
            double main_comm_modularity = modularityContributionByCommunity(*main_community, totalEdges);
            double other_comm_modularity = modularityContributionByCommunity(*other_community, totalEdges);
            if (main_comm_modularity + other_comm_modularity > best_modularity) {
                best_modularity = main_comm_modularity + other_comm_modularity;
                if (main_community == &comm1) {
                    best_comm1_nodes = main_community->nodes;
                    best_comm2_nodes = other_community->nodes;
                } else {
                    best_comm1_nodes = other_community->nodes;
                    best_comm2_nodes = main_community->nodes;
                }
            }
        }
    }

    // Update Communities
    updateCommunity(comm1, best_comm1_nodes);
    updateCommunity(comm2, best_comm2_nodes);
}

void ApproximateCommunityDetection::updateKCommunityInformation(
    Node* node_moved,
    Community* main_community,
    Community* other_community,
    unordered_set<int>& frozen_node_ids
) {
    // Update relevant information
    for (auto& edge: node_moved->edgeList) {
        Node* neighbor = edge.first;
        int edge_weight = edge.second;

        // Find if neighbor is a frozen node
        bool is_frozen_node = (frozen_node_ids.find(neighbor->id) != frozen_node_ids.end()) ? true: false;

        if (neighbor->label == main_community->id) {
            if (!is_frozen_node) {
                // Update node removal priority queue
                vector<string> all_keys = other_community->node_removal_priority_queue.getAllKeys(to_string(neighbor->id));
                for (auto& key: all_keys) {
                    double old_value = main_community->node_removal_priority_queue.getValue(key);
                    main_community->node_removal_priority_queue.deleteElement(key);
                    main_community->node_removal_priority_queue.insertElement(key, old_value + 2.0 * edge_weight);
                }
            }

            // Update e_in, e_out for main community
            main_community->e_in -= edge_weight;
            main_community->e_out += edge_weight;
        } else {
            main_community->e_out -= edge_weight;
        }
        if (neighbor->label == other_community->id) {
            if (!is_frozen_node) {
                // Update node removal priority queue
                vector<string> all_keys = other_community->node_removal_priority_queue.getAllKeys(to_string(neighbor->id));
                for (auto& key: all_keys) {
                    double old_value = other_community->node_removal_priority_queue.getValue(key);
                    other_community->node_removal_priority_queue.deleteElement(key);
                    other_community->node_removal_priority_queue.insertElement(key, old_value - 2.0 * edge_weight);
                }
            }

            // Update e_in, e_out for other community
            other_community->e_in += edge_weight;
            other_community->e_out -= edge_weight;
        } else {
            other_community->e_out += edge_weight;
        }
    }
}

void ApproximateCommunityDetection::updateCommunity(Community& comm, unordered_set<Node*> comm_node_list) {
    // Update community nodes
    comm.nodes = comm_node_list;
    comm.node_removal_priority_queue = HeapAndMap();

    // Update node labels
    for (auto& node: comm.nodes) {
        node->label = comm.id;
    }

    // Update e_in, e_out
    comm.e_in = 0;
    comm.e_out = 0;
    for (const auto& node: comm_node_list) {
        for (const auto& edge: node->edgeList) {
            if (edge.first->label == node->label) {
                comm.e_in += edge.second;
            } else {
                comm.e_out += edge.second;
            }
        }
    }
    comm.e_in /= 2;
}

bool ApproximateCommunityDetection::runKFMAlgorithm() {
    bool no_node_moved = true;

    createKHeapAndMap();

    unordered_set<int> frozen_node_ids;
    double best_modularity = newmansModularity(communities, totalEdges);
    unordered_map<int, unordered_set<Node*>> best_communities;
    for (auto& comm: communities) {
        best_communities[comm.first] = comm.second.nodes;
    }

    while (!allCommunitiesQueueEmpty()) {
        Community* main_comm = getMainCommunity();

        // Stop if no main community is found
        if (main_comm == nullptr) {
            break;
        }

        // Get top elements from heapAndMap
        string node_info = main_comm->node_removal_priority_queue.getMaxElementId();
        if (node_info == "null") {
            break;
        }
        int node_id = stoi(node_info.substr(0, node_info.find("_")));
        int other_comm_id = stoi(node_info.substr(node_info.find("_") + 1));
        Node* node_moved = acd_graph.getNode(node_id);

        // Get next relevant community
        Community* other_comm = &communities.at(other_comm_id);

        // Remove node from main community
        vector<string> all_keys = main_comm->node_removal_priority_queue.getAllKeys(to_string(node_id));
        for (auto& key: all_keys) {
            main_comm->node_removal_priority_queue.deleteElement(key);
        }
        main_comm->nodes.erase(node_moved);
        other_comm->nodes.insert(node_moved);
        node_moved->label = other_comm->id;

        // Update frozen node ids
        frozen_node_ids.insert(node_moved->id);

        // Update relevant community information
        updateKCommunityInformation(node_moved, main_comm, other_comm, frozen_node_ids);

        if (allCommunitiesSameSize()) {
            // Calculate overall modularity
            double current_modularity = newmansModularity(communities, totalEdges);
            if (current_modularity > best_modularity) {
                no_node_moved = false;
                best_modularity = current_modularity;
                for (auto& comm: communities) {
                    best_communities[comm.first] = comm.second.nodes;
                }
            }
        }
    }

    // Update Communities
    for (auto& comm: best_communities) {
        updateCommunity(communities.at(comm.first), comm.second);
    }

    return no_node_moved;
}

bool ApproximateCommunityDetection::allCommunitiesQueueEmpty() {
    for (auto& comm: communities) {
        if (!comm.second.node_removal_priority_queue.isEmpty()) {
            return false;
        }
    }
    return true;
}

bool ApproximateCommunityDetection::allCommunitiesSameSize() {
    size_t size = communities.begin()->second.nodes.size();
    for (auto& comm: communities) {
        if (comm.second.nodes.size() != size) {
            return false;
        }
    }
    return true;
}

Community* ApproximateCommunityDetection::getMainCommunity() {
    size_t max_size = 0;
    double max_priority = -numeric_limits<double>::infinity();
    Community* main_community = nullptr;

    // Find community with maximum priority
    for (auto& comm: communities) {
        if (comm.second.nodes.size() > max_size) {
            max_size = comm.second.nodes.size();
            main_community = &comm.second;
            max_priority = comm.second.node_removal_priority_queue.getMaxValue();
        } else if (comm.second.nodes.size() == max_size) {
            // Break ties using priority
            double priority = comm.second.node_removal_priority_queue.getMaxValue();
            if (priority > max_priority) {
                main_community = &comm.second;
                max_priority = priority;
            }
        }
    }

    return main_community;
}

// TODO: Check for correctness
void ApproximateCommunityDetection::createKHeapAndMap() {
    // Calculate community degree
    unordered_map<int, double> community_degrees;
    for (const auto& comm: communities) {
        double degree = 0.0;
        for (const auto& node: comm.second.nodes) {
            degree += node->degree;
        }
        community_degrees[comm.first] = degree;
    }

    // Calculate modularity gains for each community pair
    for (auto& main_comm: communities) {
        vector<pair<string, double>> modularity_gains;
        for (const auto& node: main_comm.second.nodes) {
            unordered_map<int, double> node_community_contribution;
            node_community_contribution[main_comm.first] = 0.0;
            for (const auto& edge: node->edgeList) {
                Node* neighbor = edge.first;
                double edge_weight = static_cast<double>(edge.second);
                node_community_contribution[neighbor->label] += edge_weight;
            }

            double degree_squared = static_cast<double>(node->degree * (community_degrees.at(main_comm.first) - node->degree)) / (2.0 * totalEdges);
            double current_community_loss = node_community_contribution.at(main_comm.first) - degree_squared;

            for (const auto& comm: communities) {
                if (comm.first == main_comm.first) {
                    continue;
                }
                double gain = 0.0;
                try {
                    gain += node_community_contribution.at(comm.first);
                } catch (const out_of_range&) {
                    gain = 0.0;
                }
                gain -= static_cast<double>(node->degree * community_degrees.at(comm.first)) / (2.0 * totalEdges);
                gain -= current_community_loss;

                modularity_gains.emplace_back(to_string(node->id) + "_" + to_string(comm.first), gain);
            }
        }
        main_comm.second.node_removal_priority_queue.populateHeapAndMap(modularity_gains);
    }
}
