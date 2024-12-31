#ifndef HEAP_AND_MAP_H
#define HEAP_AND_MAP_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>

using namespace std;

class HeapAndMap {
    private:
        int heapSize;
        vector<pair<string, double>> heapArray;
        unordered_map<string, int> id_index_map;

        void createMap();
        void buildHeap();
        void heapifyUp(int index);
        void heapifyDown(int index);
        void swapAndUpdate(int i, int j);
        void deleteElementByIndex(int index);

    public:
        HeapAndMap();
        ~HeapAndMap();

        void populateHeapAndMap(vector<pair<string, double>> array);
        void insertElement(string elementId, double value);
        void deleteElement(string elementId);
        void popElement();
        string getMaxElementId();
        void printInfo();
        bool isEmpty();
        double getMaxValue();
        double getValue(string elementId);
        vector<string> getAllKeys(string elementId);
};

#endif // HEAP_AND_MAP_H
