#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>
#include "x3WTRBTrieIntersection.hpp"
#include "x3WTRBinaryTrie.hpp"

using namespace std;
using namespace sdsl;

bool verbose = false;
bool parallel = false;
bool balance = false;
uint32_t block_size = 512; //Only needed on binTrie_il
uint16_t wsize = 64;


vector<vector<uint64_t>> loadQueryLog(string queryPath, uint64_t n){
    vector<vector<uint64_t>> queries;
    ifstream in(queryPath);
    if (!in.is_open()){
        cerr << "Can't open file: " << queryPath << "\n";
        return queries;
    }
    string line;
    while(getline(in, line)) {
        vector<uint64_t> query;
        istringstream iss(line);
        for (string s; iss >> s;) {
            uint64_t id = (uint64_t)stoull(s);
            if(id < n)
                query.push_back(id);
        }
        if(query.size() > 1)
            queries.push_back(query);
    }
    in.close();
    return queries;
}

template<typename rankType, typename wordType>
map<uint64_t, x3WTRBinaryTrie<rankType, wordType>*> loadTries(ifstream &in, vector<vector<uint64_t>> &queries, uint64_t n){
    vector<uint64_t> setIndexes;
    for(auto q: queries)
        setIndexes.insert(setIndexes.end(), q.begin(), q.end());
    sort(setIndexes.begin(), setIndexes.end());
    setIndexes.erase(unique(setIndexes.begin(), setIndexes.end()), setIndexes.end());

    map<uint64_t, x3WTRBinaryTrie<rankType, wordType>*> tries; 
    uint64_t nIl = 0;
    uint32_t np = 0;
    for(uint32_t i = 0; i < n; ++i) {
        x3WTRBinaryTrie<rankType, wordType>* trie = new x3WTRBinaryTrie<rankType, wordType>();
        trie -> load(in);
        if (i == setIndexes[np]){
            tries.insert({i, trie});
            np++;
            if (np == setIndexes.size()) break;
        }
        else { // I dont now how space will use the trie
            delete trie;
        }
    }
    return tries;
}

template<typename rankType, typename wordType>
void performIntersections( std::ifstream &in_sequences, std::string query_path,
                           std::string out_path, bool parallel, bool balance) {
    uint16_t rep = 5;
    std::ofstream out;
    if (out_path != "") {
        out.open(out_path, std::ios::out);
        out << "query_number,elements_per_query,time execution,size_intersection" << std::endl; 
    }

    vector<vector<uint64_t>> queries;
    map<uint64_t, x3WTRBinaryTrie<rankType, wordType>*> tries;


    uint32_t _1, n;
    uint64_t u = 0;
    in_sequences.read(reinterpret_cast<char*>(&n), sizeof(n));
    in_sequences.read(reinterpret_cast<char*>(&_1), sizeof(_1));
    in_sequences.read(reinterpret_cast<char*>(&u), sizeof(u));
    std::cout << "Num. of sets: " << n << std::endl;
    std::cout << "Universe: "<< u << std::endl;

    queries = loadQueryLog(query_path, n);
    cout << "Queries loaded succefully, Total: " << queries.size() << "" << endl;
    tries = loadTries<rankType, wordType>(in_sequences, queries, n);
    cout << "Sequences loaded succefully, Total: " << tries.size() << endl;

    cout << "Computing queries...\n";
    uint64_t nq = 0;
    uint64_t size_all_intersections = 0;
    long unsigned int total_time = 0;
    for (auto q: queries) {
        vector<x3WTRBinaryTrie<rankType, wordType>*> QTries;
        for (auto i: q){
            vector<uint64_t> decodedTrie;
            QTries.push_back(tries[i]);            
        }

        vector<uint64_t> intersection;
        if (QTries.size() <= 16){
            long unsigned int part_time = 0;  
            for (uint16_t i = 0; i < rep; ++i){
                auto start = std::chrono::high_resolution_clock::now();
                intersection = Intersect<rankType, wordType>(QTries, parallel, balance);
                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                auto time = elapsed.count();
                total_time += time;
                part_time += time;
                if (i !=rep-1)
                    intersection.clear();
            }
            
            if (out.is_open()) {
                out << nq << "," << QTries.size() << "," 
                    <<  (double)(part_time*1e-3)/rep << "," 
                    << intersection.size() 
                    << std::endl;
            }
            size_all_intersections += intersection.size();
            intersection.clear();
            ++nq;
            if (nq % 1000 == 0 && verbose) {
                std::cout << nq << " correct queries processed" << std::endl;
            }
        }
        
    }
    for (auto T: tries)
        delete T.second;

    std::cout << "Number of queries: " << nq << std::endl;
    std::cout << "Total size of intersections: " << size_all_intersections << std::endl;
    std::cout << "Avg size of intersections: " << (double)size_all_intersections/nq << std::endl;
    std::cout <<"Avg time execution: " << (double)(total_time*1e-3)/(nq*rep) << "[ms]" << std::endl;
    std::cout << "---------------------------------------------------------\n";
}


int main(int argc, char const *argv[]) {
    int mandatory = 3;
    std::string output_filename = "";
    // (*) mandatory parameters
    if (argc < mandatory){
        std::cout   << "collection filename " // (*)
                        "query log" // (*)
                    << "\n";
        return 1;
    }


    for (int i = 1; i < argc; ++i){
        if (std::string(argv[i]) == "--verbose") {
            verbose = true;
        }
        if (std::string(argv[i]) == "--parallel"){
            ++i;
            if (std::string(argv[i]) == "t")
                parallel = true;
            else 
                parallel = false;
        }
        if (std::string(argv[i]) == "--balance"){
            ++i;
            if (std::string(argv[i]) == "t")
                balance = true;
            else 
                balance = false;
        }
        if (std::string(argv[i]) == "--out") {
            ++i;
            output_filename = std::string(argv[i]);
        }
    }

    int rankT = 0;
    uint64_t min_size;
    std::string sequences_filename = std::string(argv[1]);
    std::string querylog_filename   = std::string(argv[2]);

    ifstream in_sequences;
    in_sequences.open(sequences_filename, std::ios::binary | std::ios::in);

    in_sequences.read(reinterpret_cast<char *>(&rankT) ,sizeof(rankT));
    if (rankT == 1) 
        in_sequences.read(reinterpret_cast<char *>(&block_size), sizeof(block_size));
    in_sequences.read(reinterpret_cast<char *>(&wsize), sizeof(wsize));

    std::cout << "Type of trie\n";
    std::cout << "* Rank: ";
    if (rankT == 0) std::cout << "rank v" << std::endl;
    else if (rankT == 1) {
        std::cout << "rank il" << std::endl;
        std::cout << "** Block size: " << block_size << std::endl;
    }
    else std::cout << "rank v5" << std::endl;
    std::cout << "* Word size: " << wsize << std::endl;
    std::cout << "Output file: " << output_filename << endl;

    if (rankT == 0){
        if (wsize == 64)
            performIntersections<sdsl::rank_support_v<1>, uint64_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else if (wsize == 32)
            performIntersections<sdsl::rank_support_v<1>, uint32_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else if (wsize == 16)
            performIntersections<sdsl::rank_support_v<1>, uint16_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else
            performIntersections<sdsl::rank_support_v<1>, uint8_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
    }
    else{
        if (wsize == 64)
            performIntersections<sdsl::rank_support_v5<1>, uint64_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else if (wsize == 32)
            performIntersections<sdsl::rank_support_v5<1>, uint32_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else if (wsize == 16)
            performIntersections<sdsl::rank_support_v5<1>, uint16_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
        else
            performIntersections<sdsl::rank_support_v5<1>, uint8_t>(in_sequences, querylog_filename, output_filename, parallel, balance);
    }
        
    return 0;
}