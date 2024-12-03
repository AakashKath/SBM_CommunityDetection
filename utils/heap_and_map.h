#ifndef HEAP_AND_MAP_H
#define HEAP_AND_MAP_H

#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class HeapAndMap {
    private:
        int heapSize;
        vector<pair<int, double>> heapArray;
        unordered_map<int, int> id_index_map;

        void createMap();
        void buildHeap();
        void heapifyUp(int index);
        void heapifyDown(int index);
        void swapAndUpdate(int i, int j);
        void deleteElementByIndex(int index);

    public:
        HeapAndMap();
        ~HeapAndMap();

        void populateHeapAndMap(vector<pair<int, double>> array);
        void insertElement(int elementId, double value);
        void deleteElement(int elementId);
        void popElement();
        int getMaxElementId();
        void printInfo();
};

#endif // HEAP_AND_MAP_H
