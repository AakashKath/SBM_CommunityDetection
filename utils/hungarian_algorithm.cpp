#include "hungarian_algorithm.h"

HungarianAlgorithm::HungarianAlgorithm(vector<vector<int>> costs): costMatrix(costs) {
    if (costs.empty() || costs[0].size() != costs.size()) {
        throw invalid_argument("Cost matrix must be non-empty and square.");
    }
    size = costMatrix.size();
}

HungarianAlgorithm::~HungarianAlgorithm() {
    // Nothing to clean
}

// Solves the assignment problem and returns the minimum cost
int HungarianAlgorithm::solveAssignmentProblem() {
    int totalCost = 0;
    matches = 0;
    matchX = vector<int>(size, -1);
    matchY = vector<int>(size, -1);
    initializeLabels();
    augment();  // Perform augmentation until all workers are matched

    // Calculate total cost of the optimal assignment
    for (int worker = 0; worker < size; worker++) {
        totalCost += costMatrix[worker][matchX[worker]];
    }

    printMatching();

    return totalCost;
}

// Initializes the label values for workers and jobs
void HungarianAlgorithm::initializeLabels() {
    labelX = vector<int>(size, 0);
    labelY = vector<int>(size, 0);
    for (int worker = 0; worker < size; worker++) {
        for (int job = 0; job < size; job++) {
            labelX[worker] = max(labelX[worker], costMatrix[worker][job]); // Set initial worker labels
        }
    }
}

// Augment the matching using BFS
void HungarianAlgorithm::augment() {
    if (matches == size) return;  // If all workers are matched, stop

    int worker, job, root;
    queue<int> bfsQueue;

    initializeBFS(root, bfsQueue);
    if (bfsQueue.empty()) return;  // No unmatched worker found, exit

    initializeSlack(root);

    while (true) {
        if (findAugmentingPath(bfsQueue, worker, job)) break;

        updateLabels();
        queue<int>().swap(bfsQueue);  // Clear queue

        if (extendAlternatingTree(bfsQueue, worker, job)) break;
    }

    updateMatching(worker, job);
    augment();  // Recursively call augment until all matches are made
}

// Initializes BFS search for augmenting paths
void HungarianAlgorithm::initializeBFS(int &root, queue<int> &bfsQueue) {
    inTreeX = vector<int>(size, false);
    inTreeY = vector<int>(size, false);
    parent = vector<int>(size, -1);

    // Find the first unmatched worker
    for (int worker = 0; worker < size; worker++) {
        if (matchX[worker] == -1) {
            root = worker;
            parent[worker] = -2;  // Mark root node
            inTreeX[worker] = true;
            bfsQueue.push(worker); // Push root worker to BFS queue
            return;  // Stop after finding the first unmatched worker
        }
    }
}

// Initializes slack values for potential jobs
void HungarianAlgorithm::initializeSlack(int root) {
    slack = vector<int>();
    slackX = vector<int>();
    for (int job = 0; job < size; job++) {
        slack.push_back(labelX[root] + labelY[job] - costMatrix[root][job]);
        slackX.push_back(root);
    }
}

// Performs BFS to find an augmenting path
bool HungarianAlgorithm::findAugmentingPath(queue<int> &bfsQueue, int &worker, int &job) {
    while (!bfsQueue.empty()) {
        worker = bfsQueue.front();
        bfsQueue.pop();

        for (job = 0; job < size; job++) {
            if (!inTreeY[job] && costMatrix[worker][job] == labelX[worker] + labelY[job]) { // Found a tight edge
                if (matchY[job] == -1) return true;  // Found an unmatched job
                inTreeY[job] = true;
                bfsQueue.push(matchY[job]);
                addToTree(matchY[job], worker);
            }
        }
    }
    return false;
}

// Adds a worker to the alternating tree and updates slack values
void HungarianAlgorithm::addToTree(int worker, int prevX) {
    inTreeX[worker] = true;
    parent[worker] = prevX;

    for (int job = 0; job < size; job++) {
        int reducedCost = labelX[worker] + labelY[job] - costMatrix[worker][job];
        if (reducedCost < slack[job]) {
            slack[job] = reducedCost;
            slackX[job] = worker;
        }
    }
}

// Updates labels to maintain feasibility of matching
void HungarianAlgorithm::updateLabels() {
    int delta = INT_MAX;

    // Find the minimum slack value among all unselected jobs
    for (int job = 0; job < size; job++) {
        if (!inTreeY[job]) {
            delta = min(delta, slack[job]);
        }
    }

    // Reduce labelX for selected workers
    for (int worker = 0; worker < size; worker++) {
        if (inTreeX[worker]) {
            labelX[worker] -= delta;
        }
    }

    // Increase labelY for selected jobs and update slack values
    for (int job = 0; job < size; job++) {
        if (inTreeY[job]) {
            labelY[job] += delta;
        } else {
            slack[job] -= delta;
        }
    }
}

// Extends the alternating tree by adding new tight edges
bool HungarianAlgorithm::extendAlternatingTree(queue<int> &bfsQueue, int &worker, int &job) {
    for (job = 0; job < size; job++) {
        if (!inTreeY[job] && slack[job] == 0) {  // Check new tight edges
            if (matchY[job] == -1) {  // Found an unmatched job
                worker = slackX[job];
                return true;
            } else {
                inTreeY[job] = true;
                if (!inTreeX[matchY[job]]) {
                    bfsQueue.push(matchY[job]);
                    addToTree(matchY[job], slackX[job]);
                }
            }
        }
    }
    return false;
}

// Updates the matching along an augmenting path
void HungarianAlgorithm::updateMatching(int worker, int job) {
    matches++; // Increase number of matches
    int current_worker = worker, current_job = job, tempY;

    // Flip matched edges along the augmenting path
    do {
        tempY = matchX[current_worker];
        matchY[current_job] = current_worker;
        matchX[current_worker] = current_job;
        current_worker = parent[current_worker];
        current_job = tempY;
    } while (current_worker != -2);
}

void HungarianAlgorithm::printMatching() {
    cout << "Worker-to-Job Matching:\n";
    for (int i = 0; i < size; i++) {
        cout << "Worker " << i << " -> Job " << matchX[i] << endl;
    }
}
