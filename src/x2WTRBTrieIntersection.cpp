#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <bitset>
#include <math.h>
#include <algorithm>
#include "x2WTRBinaryTrie.hpp"
#ifdef LINUX
#include "parallel_for.hpp"
#elif MACOS
#include "parallel_for_macos.hpp"
#else
#error No operating system supported to parallel for
#endif


using namespace std;


struct IndexWeight {
    size_t index;
    uint32_t lessWeight;
    
    bool operator<(const IndexWeight& other) const {
        return lessWeight < other.lessWeight; // Ordenar por peso
    }
};


template<typename rankType, typename wordType>
bool AND(vector<x2WTRBinaryTrie<rankType, wordType>*> &Ts, uint64_t nTries, uint64_t &maxLevel,
        uint64_t currLevel, uint64_t roots[], uint64_t prefix, vector<uint64_t> &r, uint64_t posTrie[16]){
    if (currLevel == maxLevel-1) {
        wordType w = (~0);
        for (uint16_t i = 0; i < nTries; ++i) 
            w &= Ts[i] -> getTrieLevelPos(posTrie[i]) -> getWordHigh(roots[i]);
        while (w != 0){
            wordType t = w & (~w + 1);            
            uint64_t z = __builtin_ctzll(w);
            // Create a mask with bits set up to the given position
            uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
            //  AND at the low part
            wordType wLow = (~0);
            for (uint16_t j = 0; j < nTries; ++j) {
                // Apply the mask to the number
                uint64_t masked_number = (uint64_t)Ts[j] -> getTrieLevelPos(posTrie[j]) -> getWordHigh(roots[j]) & mask;
                //Count the number of bits set to 1
                uint64_t pos = __builtin_popcountll(masked_number) -1;
              
                wLow &= Ts[j] -> getTrieLevelPos(posTrie[j]) -> getWordLow(roots[j], pos);
            }

            while (wLow != 0){
                wordType tLow = wLow & (~wLow + 1);            
                uint64_t zLow = __builtin_ctzll(wLow);
                r.push_back((z * sizeof(wordType)*8 + zLow) + prefix);
                
                wLow ^= tLow;
            }
            w ^= t;
        }
        return true;
    }

    uint64_t i;
    uint64_t andResult = 0b11;
    for (i = 0; i < nTries; ++i) {
        uint64_t node_i = Ts[i] -> getTrieLevelPos(posTrie[i]) -> getNode(roots[i], currLevel);
        andResult &= node_i;
        
    }
    // Can't go any further down in that branch.
    if (andResult == 0b00)
        return false;

    bool existLeft, existRight;
    uint64_t nextLevel = currLevel + 1;
    if(andResult == 0b10) {
        uint64_t leftNodes[16];
        uint64_t leftResult = prefix;
        for (i = 0; i < nTries; ++i) {
            if (currLevel != maxLevel - 1) 
                leftNodes[i] = Ts[i] -> getTrieLevelPos(posTrie[i]) -> getLeftChild(roots[i], currLevel);
        }
        existLeft = AND<rankType>(Ts, nTries, maxLevel, nextLevel, leftNodes, leftResult, r, posTrie);
    }
    else if (andResult == 0b01) {
        uint64_t rightNodes[16];
        uint64_t rightResult = (prefix | ((uint64_t)1 << ((maxLevel+(uint64_t)(log2(pow(sizeof(wordType)*8,2)))-1)- currLevel - 1)));
        for (i = 0; i < nTries; ++i) {
            if (currLevel != maxLevel -1)
                rightNodes[i] = Ts[i] -> getTrieLevelPos(posTrie[i]) -> getRightChild(roots[i], currLevel);
        }
        existRight = AND<rankType>(Ts, nTries, maxLevel, nextLevel, rightNodes, rightResult, r, posTrie);

    }
    else if (andResult == 0b11) {
        uint64_t leftNodes[16];
        uint64_t rightNodes[16];
        uint64_t leftResult = prefix;
        uint64_t rightResult = (prefix | ((uint64_t)1 << ((maxLevel+(uint64_t)(log2(pow(sizeof(wordType)*8,2)))-1)- currLevel - 1)));
        for (i = 0; i < nTries; ++i) {
            if (currLevel != maxLevel - 1) {
                uint64_t leftNode = Ts[i] -> getTrieLevelPos(posTrie[i]) -> getLeftChild(roots[i], currLevel);
                leftNodes[i] = leftNode;
                rightNodes[i] = leftNode + 1;
            }
        }
        existLeft = AND<rankType>(Ts, nTries, maxLevel, nextLevel, leftNodes, leftResult, r, posTrie);
        existRight = AND<rankType>(Ts, nTries, maxLevel, nextLevel, rightNodes, rightResult, r, posTrie);
    }
    if (existLeft || existRight)
        return true;
    return false;
};

template<typename rankType, typename wordType>
void IntersectPartition(vector<x2WTRBinaryTrie<rankType, wordType>*> &Ts, uint64_t &nTries, uint64_t &height, vector<uint64_t> &groupTrieP, uint64_t groupTrieA[64][16], vector<uint64_t> &intersection){

    vector<uint64_t> intersectionAux;
    intersectionAux.reserve(1000000);
    for (uint16_t j = 0; j < groupTrieP.size(); ++j) { 
        uint64_t roots[16] = { 0 };
        AND<rankType>(Ts, nTries, height, 0, roots, 0, intersectionAux, groupTrieA[groupTrieP[j]]);
        intersection.insert(intersection.end(), intersectionAux.begin(), intersectionAux.end());
        intersectionAux.clear();
    }
}

// Distribución balanceada según el peso
void distributeBalanced(const vector<IndexWeight>& arr, int k, vector<vector<size_t>>& groups) {
    int n = arr.size();
    std::vector<uint32_t> groupSums(k, 0); // Sumas de cada grupo
    for (int i = n - 1; i >= 0; --i) {
        // Encuentra el grupo con la menor suma actual
        int minGroup = 0;
        for (int j = 1; j < k; ++j) {
            if (groupSums[j] < groupSums[minGroup]) {
                minGroup = j;
            }
        }
        // Asigna el índice del elemento al grupo con la menor suma
        groups[minGroup].push_back(arr[i].index);
        groupSums[minGroup] += arr[i].lessWeight;
    }
}

// Distribución por orden secuencial
void distributeSequential(const vector<IndexWeight>& arr, int k, vector<vector<size_t>>& groups) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        int groupIndex = i % k; // Asignar secuencialmente al grupo correspondiente
        groups[groupIndex].push_back(arr[i].index);
    }
}

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<x2WTRBinaryTrie<rankType, wordType>*> &Ts, bool parallel, bool balance){
        // Hint of threads avaible in our CPU
    unsigned nb_threads = std::thread::hardware_concurrency();
    uint64_t zero = 0;
    uint64_t heightU = Ts[0] -> getHeightU();    
    uint64_t height = Ts[0] -> getTrieLevelPos(zero) -> getHeight();
    wordType wFLevel (~0);
    for (auto T: Ts){
        if (T->getHeightU() != heightU){
            cerr << "All tries need to be of same height\n"; 
            return vector<uint64_t>(); // return empty vector
        }
        wFLevel &= T->getFirstLevel();
    }

    uint64_t nTries = Ts.size();
    vector<uint64_t> intersection;
    intersection.reserve(1000000);   
    uint32_t nTrieLevel = 0;
    if (sizeof(wordType) == sizeof(uint64_t)) {
        nTrieLevel = __builtin_popcountll(static_cast<uint64_t>(wFLevel));
    } else {
        nTrieLevel = __builtin_popcount(static_cast<uint32_t>(wFLevel));
    }

    uint64_t groupTrieA[64][16] = {0};
    size_t index = 0;
    if (wFLevel != 0) {
        if (parallel) {
            vector<IndexWeight> indicesWeight;
            while (wFLevel != 0) {
                wordType t = wFLevel & (~wFLevel + 1); // Obtiene el bit menos significativo activo
                uint64_t z = __builtin_ctzll(wFLevel); // Encuentra la posición del bit menos significativo activo
                uint32_t lessWeight = 0;
                uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;            
                for (uint16_t j = 0; j < nTries; ++j) {                    
                    uint64_t masked_number = (uint64_t)Ts[j]->getFirstLevel() & mask;
                    uint64_t pos = __builtin_popcountll(masked_number) - 1;
                    groupTrieA[index][j] = pos;
                    if(lessWeight == 0 || lessWeight > Ts[j]->getTrieSizePos(pos)) {
                        lessWeight = Ts[j]->getTrieSizePos(pos);
                    }                
                }

                indicesWeight.push_back({index,lessWeight});
                ++index;

                wFLevel ^= t;
            }
            vector<vector<size_t>> groupTrieP(nb_threads);
            if (balance){            
                std::sort(indicesWeight.begin(), indicesWeight.end());
                distributeBalanced(indicesWeight, nb_threads, groupTrieP);
            } else {
                distributeSequential(indicesWeight, nb_threads, groupTrieP);
            }
            // Init the vector to contain threads solutions
            vector<vector<uint64_t>> threads_results(nb_threads);
            for (auto tr: threads_results)
                tr.reserve(1000000);
// cout<<"groupTrieP: " <<groupTrieP.size()<<", nb_threads: "<< nb_threads<<endl;
            parallel_for(nb_threads, nb_threads, [&](int start, int end) {
                if(start < end) {
                    IntersectPartition<rankType, wordType>(Ts, nTries, height, groupTrieP[start], groupTrieA, threads_results[start]);
                }
            });

            uint64_t output_size = 0; 
            vector<uint64_t> shifts(nb_threads);
            uint64_t shift = 0;
            for(uint16_t t = 0; t < nb_threads; ++t){            
                output_size += threads_results[t].size();
                shifts[t] = shift;
                shift += threads_results[t].size();
            }
            // Write in parallel threads result
            if (output_size > 450000){
                intersection.resize(output_size);
                parallel_for(nb_threads, nb_threads, [&](int start, int end) {
                    for (uint16_t threadId = start; threadId < end; ++threadId) {
                        for (uint64_t j = 0; j < threads_results[threadId].size(); ++j) {
                            intersection[j+shifts[threadId]] = threads_results[threadId][j];
                        } 
                    }        
                });
            } 
            else {
                // Concatenate solutions of threads
                for(uint64_t t=0; t < nb_threads; ++t){
                    intersection.insert(intersection.end(), 
                                        threads_results[t].begin(),
                                        threads_results[t].end()
                                        );
                }            
            }
        } else {
            while (wFLevel != 0) {
                wordType t = wFLevel & (~wFLevel + 1); // Obtiene el bit menos significativo activo
                uint64_t z = __builtin_ctzll(wFLevel); // Encuentra la posición del bit menos significativo activo
                uint32_t lessWeight = 0;
                uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                for (uint16_t j = 0; j < nTries; ++j) {
                    uint64_t masked_number = (uint64_t)Ts[j]->getFirstLevel() & mask;
                    uint64_t pos = __builtin_popcountll(masked_number) - 1;
                    groupTrieA[index][j] = pos;               
                }
                ++index;
                wFLevel ^= t;
            }
            vector<uint64_t> intersectionAux;
            intersectionAux.reserve(1000000);
            for (uint16_t j = 0; j < nTrieLevel; ++j) {    
                uint64_t roots[16] = { 0 };
                AND<rankType>(Ts, nTries, height, 0, roots, 0, intersectionAux, groupTrieA[j]);
                intersection.insert(intersection.end(), intersectionAux.begin(), intersectionAux.end());
                intersectionAux.clear();
            }
        }

    }

    return intersection;
}
// Rank v
template
vector<uint64_t> Intersect<rank_support_v<1>, uint64_t>(vector<x2WTRBinaryTrie<rank_support_v<1>, uint64_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v<1>, uint32_t>(vector<x2WTRBinaryTrie<rank_support_v<1>, uint32_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v<1>, uint16_t>(vector<x2WTRBinaryTrie<rank_support_v<1>, uint16_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v<1>, uint8_t>(vector<x2WTRBinaryTrie<rank_support_v<1>, uint8_t>*> &Ts, bool parallel, bool balance);
// Rank v5
template
vector<uint64_t> Intersect<rank_support_v5<1>, uint64_t>(vector<x2WTRBinaryTrie<rank_support_v5<1>, uint64_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v5<1>, uint32_t>(vector<x2WTRBinaryTrie<rank_support_v5<1>, uint32_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v5<1>, uint16_t>(vector<x2WTRBinaryTrie<rank_support_v5<1>, uint16_t>*> &Ts, bool parallel, bool balance);
template
vector<uint64_t> Intersect<rank_support_v5<1>, uint8_t>(vector<x2WTRBinaryTrie<rank_support_v5<1>, uint8_t>*> &Ts, bool parallel, bool balance);