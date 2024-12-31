#include "heap_and_map.h"


HeapAndMap::HeapAndMap(): heapArray(), heapSize(0) {
    // Default constructor
}

void HeapAndMap::populateHeapAndMap(vector<pair<string, double>> array) {
    heapArray = move(array);
    heapSize = heapArray.size();
    createMap();
    buildHeap();
}

HeapAndMap::~HeapAndMap() {
    // Nothing to clean
}

void HeapAndMap::createMap() {
    id_index_map.clear();
    for (int i = 0; i < heapSize; ++i) {
        id_index_map[heapArray[i].first] = i;
    }
}

void HeapAndMap::buildHeap() {
    for (int i = (heapSize - 1) / 2; i >= 0; --i) {
        heapifyDown(i);
    }
}

void HeapAndMap::swapAndUpdate(int i, int j) {
    // Validate indices
    if (i < 0 || i >= heapSize || j < 0 || j >= heapSize) {
        cerr << "swapAndUpdate: Index out of bounds (" << i << ", " << j << ")" << endl;
        return;
    }

    // Swap elements in the heap
    swap(heapArray[i], heapArray[j]);

    // Update index in map
    id_index_map[heapArray[i].first] = i;
    id_index_map[heapArray[j].first] = j;
}

void HeapAndMap::heapifyDown(int index) {
    // Validate index
    if (index < 0 || index >= heapSize) {
        cerr << "heapifyDown: Index out of bounds (" << index << ")" << endl;
        return;
    }

    int largest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < heapSize && heapArray[left_child].second > heapArray[largest].second) {
        largest = left_child;
    }

    if (right_child < heapSize && heapArray[right_child].second > heapArray[largest].second) {
        largest = right_child;
    }

    if (largest != index) {
        swapAndUpdate(index, largest);
        heapifyDown(largest);
    }
}

void HeapAndMap::heapifyUp(int index) {
    // Base case or invalid index
    if (index <= 0 || index >= heapSize) {
        return;
    }

    int parent = (index - 1) / 2;
    if (heapArray[parent].second < heapArray[index].second) {
        swapAndUpdate(parent, index);
        heapifyUp(parent);
    }
}

void HeapAndMap::insertElement(string elementId, double value) {
    // Validate if element already exists
    if (id_index_map.find(elementId) != id_index_map.end()) {
        cerr << "insertElement: Element already exists with ID " << elementId << endl;
        return;
    }

    // Add the new element at the end of the array
    heapArray.emplace_back(elementId, value);
    heapSize++;

    // Update map with the new element
    int index = heapSize - 1;
    id_index_map[elementId] = index;

    // Heapify-up to maintain max-heap property
    heapifyUp(index);
}

void HeapAndMap::deleteElement(string elementId) {
    auto it = id_index_map.find(elementId);
    if (it == id_index_map.end()) {
        cerr << "deleteElement: Element not found with ID " << elementId << endl;
        return;
    }

    deleteElementByIndex(it->second);
}

void HeapAndMap::popElement() {
    if (heapSize == 0) {
        cerr << "popElement: Heap is empty." << endl;
        return;
    }

    deleteElementByIndex(0);
}

double HeapAndMap::getMaxValue() {
    if (heapSize == 0) {
        cerr << "getMaxValue: Heap empty" << endl;
        return -numeric_limits<double>::infinity();
    }

    return heapArray[0].second;
}

double HeapAndMap::getValue(string elementId) {
    auto it = id_index_map.find(elementId);
    if (it == id_index_map.end()) {
        cerr << "getValue: Element not found with ID " << elementId << endl;
        return -numeric_limits<double>::infinity();
    }

    return heapArray[it->second].second;
}

string HeapAndMap::getMaxElementId() {
    if (heapSize == 0) {
        cerr << "getMaxElementId: Heap empty" << endl;
        return "null";
    }

    return heapArray[0].first;
}

void HeapAndMap::deleteElementByIndex(int index) {
    // Validate index
    if (index < 0 || index >= heapSize) {
        cerr << "deleteElementByIndex: Index out of bounds (" << index << ")" << endl;
        return;
    }

    // Replace the element at the index with the last element
    swapAndUpdate(index, heapSize - 1);

    // Remove the last element from the heap and map
    id_index_map.erase(heapArray.back().first);
    heapArray.pop_back();
    heapSize--;

    // Heapify-down from the index to maintain max-heap property
    if (index < heapSize) {
        heapifyDown(index);
    }
}

void HeapAndMap::printInfo() {
    cout << "Heap: " << endl;
    for (const auto& pair: heapArray) {
        cout << "ID: " << pair.first << " Value: " << pair.second << endl;
    }

    cout << "Map: " << endl;
    for (const auto& pair: id_index_map) {
        cout << "ID: " << pair.first << " Index: " << pair.second << endl;
    }
}

bool HeapAndMap::isEmpty() {
    return heapSize == 0;
}

vector<string> HeapAndMap::getAllKeys(string elementId) {
    vector<string> keys;
    for (const auto& pair: heapArray) {
        // Check if the key starts with the given elementId
        if (pair.first.find(elementId) == 0) {
            keys.push_back(pair.first);
        }
    }

    return keys;
}
