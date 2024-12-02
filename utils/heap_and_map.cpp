#include "heap_and_map.h"


HeapAndMap::HeapAndMap(): heapArray(), heapSize(0) {
    // Default constructor
}

void HeapAndMap::populateHeapAndMap(vector<pair<int, double>> array) {
    heapArray = array;
    heapSize = heapArray.size();
    createMap();
    buildHeap();
}

HeapAndMap::~HeapAndMap() {
    // Nothing to clean
}

void HeapAndMap::createMap() {
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
    // Swap elements in the heap
    swap(heapArray[i], heapArray[j]);

    // Update index in map
    id_index_map[heapArray[i].first] = i;
    id_index_map[heapArray[j].first] = j;
}

void HeapAndMap::heapifyDown(int index) {
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
    // Base case reached.
    if (index == 0) {
        return;
    }

    int parent = (index - 1) / 2;
    if (heapArray[parent].second < heapArray[index].second) {
        swapAndUpdate(parent, index);
        heapifyUp(parent);
    }
}

void HeapAndMap::insertElement(int elementId, double value) {
    // Add the new element at the end of the array
    heapArray.emplace_back(elementId, value);
    heapSize++;

    // Update map with the new element
    int index = heapSize - 1;
    id_index_map[elementId] = index;

    // Heapify-up to maintain max-heap property
    heapifyUp(index);
}

void HeapAndMap::deleteElement(int elementId) {
    if (id_index_map.find(elementId) == id_index_map.end()) {
        cerr << "Element not found. Please re-check the community to be deleted from." << endl;
        return;
    }

    deleteElementByIndex(id_index_map[elementId]);
}

void HeapAndMap::popElement() {
    if (heapSize == 0) {
        cerr << "Heap already empty. See the reason of pop call." << endl;
        return;
    }

    deleteElementByIndex(0);
}

int HeapAndMap::getMaxElementId() {
    if (heapSize == 0) {
        cerr << "Heap empty" << endl;
        return -1;
    }

    return heapArray[0].first;
}

void HeapAndMap::deleteElementByIndex(int index) {
    // Replace the element at the index with the last element
    swapAndUpdate(index, heapSize - 1);

    // Remove the last element from the heap and map
    id_index_map.erase(heapArray.back().first);
    heapArray.pop_back();
    heapSize--;

    // Heapify-down from the index to maintain max-heap property
    heapifyDown(index);
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
