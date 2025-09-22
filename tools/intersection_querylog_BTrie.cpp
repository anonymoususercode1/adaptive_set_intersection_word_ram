#include <iostream>
#include <vector>
#include <fstream>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/rank_support.hpp>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>
#include "../include/binTrie.hpp"
#include "../include/flatBinTrie.hpp"
#include "../include/binTrie_il.hpp"
#include "../include/flatBinTrie_il.hpp"
#include "../include/intersection.hpp"
#include "../include/binaryTrie.hpp"


using namespace std;
using namespace sdsl;

int rankType = 10;
bool runs = false;
bool levelwise = false;
uint32_t block_size = 512; //Only needed on binTrie_il
bool verbose = false;
bool parallel = true;


map<uint64_t, binaryTrie*> loadSequences(ifstream &in, vector<vector<uint32_t>> &queries, u_int64_t n){
    vector<uint64_t> setIndexes;
    for(auto q: queries)
        setIndexes.insert(setIndexes.end(), q.begin(), q.end());
    sort(setIndexes.begin(), setIndexes.end());
    setIndexes.erase(unique(setIndexes.begin(), setIndexes.end()), setIndexes.end());

    map<uint64_t, binaryTrie*> tries; 
    uint64_t nIl = 0;
    uint32_t np = 0;
    for(uint32_t i = 0; i < n; ++i) {        
        binaryTrie* trie;
        if (rankType == 0){
            if (levelwise)
                trie = new binTrie<rank_support_v<1>>();
            else 
                trie = new flatBinTrie<rank_support_v<1>>();
        }
        else if(rankType == 1){
            if(block_size == 128){
                if (levelwise)
                    trie = new binTrie_il<128>;
                else 
                    trie = new flatBinTrie_il<128>;
            }
            if (block_size == 256){
                if (levelwise)
                    trie = new binTrie_il<256>;
                else 
                    trie = new flatBinTrie_il<256>;
            }
            if (block_size == 512){
                if (levelwise)
                    trie = new binTrie_il<512>;
                else 
                    trie = new flatBinTrie_il<512>;
            }
        }
        else{
            if (levelwise)
                trie = new binTrie<rank_support_v5<1>>;
            else
                trie = new flatBinTrie<rank_support_v5<1>>;
        }     
       
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
    in.close();
    return tries;
}

vector<vector<uint32_t>> loadQueryLog(string queryPath, uint64_t n){
    vector<vector<uint32_t>> queries;
    ifstream in(queryPath);
    if (!in.is_open()){
        cerr << "Can't open file: " << queryPath << "\n";
        return queries;
    }
    string line;
    while(getline(in, line)) {
        vector<uint32_t> query;
        istringstream iss(line);
        for (string s; iss >> s;) {
            uint32_t id = (uint32_t)stoull(s);
            if(id < n)
                query.push_back(id);
        }
        if(query.size() > 1)
            queries.push_back(query);
    }
    in.close();
    return queries;
}


void performIntersections( std::ifstream &in_sequences, std::string query_path,
                           std::string out_path, bool runs_encoded) {
    uint64_t trep = 10;
    std::ofstream out;
    if (out_path != "") {
        out.open(out_path, std::ios::out);
        out << "elements_per_query,time execution,size_intersection" << std::endl; 
    }
    
    vector<vector<uint32_t>> queries;
    // vector<binaryTrie*> sequences;
    map<uint64_t, binaryTrie*> sequences;
    uint32_t _1, u, n;
    in_sequences.read(reinterpret_cast<char*>(&n), sizeof(n));
    in_sequences.read(reinterpret_cast<char*>(&_1), sizeof(_1));
    in_sequences.read(reinterpret_cast<char*>(&u), sizeof(u));
    std::cout << "Num. of sets: " << n << std::endl;
    std::cout << "Universe: "<< u << std::endl;
    
    queries  = loadQueryLog(query_path, n);
    cout << "Queries loaded succefully, Total: " << queries.size() << "" << endl;
    sequences = loadSequences(in_sequences, queries, n);
    cout << "Sequences loaded succefully, Total: " << sequences.size() << endl;

    std::cout << "Computing queries...\n";
    uint64_t nq = 0;
    uint64_t size_all_intersections = 0;
    long unsigned int total_time = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto q: queries) {
        vector<binaryTrie*> QTries;
        // cout << "Query: ";
        for (auto i: q) {
            QTries.push_back(sequences[i]);
            // std::cout << i << " ";
        }
        // std::cout << std::endl;
        vector<uint64_t> intersection;
        if (QTries.size() <= 16 ){
            uint64_t time_10 = 0;
            for(int rep = 0; rep < trep; ++rep) {
                auto start = std::chrono::high_resolution_clock::now();
                intersection = Intersect(QTries, runs_encoded, parallel);
                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                auto time = elapsed.count();
                total_time += time;
                time_10 += time;
                if (rep != 9)
                    intersection.clear();       
            }
            if (out.is_open()) {
                out << QTries.size() << "," << (float)time_10/trep<< "," 
                    << intersection.size() 
                    << std::endl;
            }
            // std::cout << nq <<"|Time execution: " << (double)(time_10*1e-3)/(trep) << "[ms] " << intersection.size() << std::endl;
            ++nq;
            // cout << "End query: " << nq << " | Size intersection: " << intersection.size() << endl;
            size_all_intersections += intersection.size();
            if (verbose && nq % 1000 == 0) {
                std::cout << nq << " queries processed" << std::endl;
            }
        }
    }
    out.close();

    for (auto T: sequences)
        delete T.second;

    std::cout << "Number of queries: " << nq << std::endl;
    std::cout << "Total size of intersections: " << size_all_intersections << std::endl;
    std::cout << "Avg size of intersections: " << (double)size_all_intersections/nq << std::endl;
    std::cout <<"Avg time execution: " << (double)(total_time*1e-3)/(nq*trep) << "[ms]" << std::endl;
    std::cout << "---------------------------------------------------------\n";
}


int main(int argc, char const *argv[]) {
    int mandatory = 3;
    // (*) mandatory parameters
    if (argc < mandatory){
        std::cout   << "collection filename " // (*)
                        "query log "          // (*)
                        "[--out results_filename]"
                    <<
        std::endl;
        return 1;
    }

    uint64_t min_size;
    std::string sequences_filename = std::string(argv[1]);
    std::string querylog_filename   = std::string(argv[2]);
    std::string output_filename = "";

    ifstream in_sequences;
    in_sequences.open(sequences_filename, std::ios::binary | std::ios::in);
    
    for (int i = 1; i < argc; ++i){
        if (std::string(argv[i]) == "--out") {
            ++i;
            output_filename = std::string(argv[i]);
        }
        if (std::string(argv[i]) == "--verbose"){
            ++i;
            if (std::string(argv[i]) == "t")
                verbose = true;
            else 
                verbose = false;
        }
        if (std::string(argv[i]) == "--parallel"){
            ++i;
            if (std::string(argv[i]) == "t")
                parallel = true;
            else 
                parallel = false;
        }
    }

    in_sequences.read(reinterpret_cast<char *>(&rankType) ,sizeof(rankType));
    if (rankType == 1) 
        in_sequences.read(reinterpret_cast<char *>(&block_size), sizeof(block_size));
    in_sequences.read(reinterpret_cast<char *>(&runs), sizeof(runs));
    in_sequences.read(reinterpret_cast<char *>(&levelwise), sizeof(levelwise));
    std::cout << "Type of trie\n";
    std::cout << "* Rank: ";
    if (rankType == 0) std::cout << "rank v" << std::endl;
    else if (rankType == 1) {
        std::cout << "rank il" << std::endl;
        std::cout << "** Block size: " << block_size << std::endl;
    }
    else std::cout << "rank v5" << std::endl;
    std::cout << "* Runs:" << (runs == 1 ? "true" : "false") << std::endl;
    std::cout << "* Level-wise: " << (levelwise == 1 ? "true" : "false") << std::endl;
    std::cout << "Output file: " << output_filename << endl;
    performIntersections(in_sequences, querylog_filename, output_filename, runs);
        
    return 0;
}