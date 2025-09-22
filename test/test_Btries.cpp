#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <string>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/rank_support.hpp>
#include "../include/binTrie.hpp"
#include "../include/flatBinTrie.hpp"
#include "../include/binTrie_il.hpp"
#include "../include/flatBinTrie_il.hpp"
#include "../include/intersection.hpp"
#include "../include/binaryTrie.hpp"

using namespace std;
using namespace sdsl;

bool runs = false;
bool verbose = false;
bool parallel = false;


vector<uint64_t> read_inv_list(std::ifstream &input_stream, uint32_t n) {
    vector <uint64_t> il;
    il.reserve(n);
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t x;
        input_stream.read(reinterpret_cast<char *>(&x), 4);
        il.push_back((uint64_t)x);
    }
    return il;
}

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

template <uint32_t block_size>
void processCollectionAndQueries(
    string input_path,
    string query_path,
    int rank_type,
    uint64_t min_size,
    uint64_t max_size
) {
    ifstream input_stream;
    input_stream.open(input_path, std::ios::binary | std::ios::in);
    if (!input_stream.is_open()){
        cerr << "Can't open input file: " << input_path << endl;
        return;
    }

    uint32_t _1;
    uint32_t u = 0;
    
    input_stream.read(reinterpret_cast<char *>(&_1), sizeof(_1));
    input_stream.read(reinterpret_cast<char *>(&u), sizeof(uint32_t));
    uint32_t n_sets = 0;
    while (true) {
        uint32_t n;
        input_stream.read(reinterpret_cast<char *>(&n), 4);
        if (input_stream.eof()){
            break;
        }
        if (n > min_size && n <= max_size) {
            n_sets++;
        }
        input_stream.seekg(n*sizeof(uint32_t), ios::cur);
    }

    input_stream.clear();
    input_stream.seekg(2*sizeof(uint32_t), ios::beg);
    
    vector<vector<uint64_t>> queries = loadQueryLog(query_path, n_sets);

    
    vector<uint64_t> setIndexes;
    for(auto q: queries)
        setIndexes.insert(setIndexes.end(), q.begin(), q.end());
    sort(setIndexes.begin(), setIndexes.end());
    setIndexes.erase(unique(setIndexes.begin(), setIndexes.end()), setIndexes.end());

    map<uint64_t, binaryTrie*> tries;
    uint64_t total_size = 0;
    uint64_t total_elements = 0;
    uint64_t n_il = 0;
    uint32_t current_set_id = 0;
    uint32_t required_idx = 0;

    while (true) {
        uint32_t n;
        input_stream.read(reinterpret_cast<char *>(&n), 4);
        if (input_stream.eof()){
            break;
        }

        if (n > min_size && n <= max_size) {
            vector <uint64_t> il = read_inv_list(input_stream, n);
            
            binaryTrie* trie;   

            if (rank_type == 0) {
                trie = new flatBinTrie<rank_support_v<1>>(il, u);
            }
            else if (rank_type == 1) {      
                trie = new flatBinTrie_il<block_size>(il, u);
            }
            else {
                trie = new flatBinTrie<rank_support_v5<1>>(il, u);
            }

            if (current_set_id == setIndexes[required_idx]) {
                tries.insert({current_set_id, trie});
                required_idx++;
                total_size += trie->size_in_bytes();
            }  else { // I dont now how space will use the trie
                total_size += trie->size_in_bytes();
                delete trie;
            }  
                
            total_elements += n;
            n_il++;
        }
        else {
            input_stream.seekg(n*sizeof(uint32_t), ios::cur);
        }
        current_set_id++;
    }
    input_stream.close();

    uint64_t nq = 0;
    uint64_t size_all_intersections = 0;
    long unsigned int total_time = 0;
    uint16_t rep = 5;
    for (auto q: queries) {
        vector<binaryTrie*> QTries;
        for (auto i: q){
            if (tries.count(i))
                QTries.push_back(tries[i]);    
        }
        if (QTries.empty()) continue;

        vector<uint64_t> intersection;
        if (QTries.size() <= 16 ){
            long unsigned int part_time = 0;  
            for (uint16_t i = 0; i < rep; ++i){
                auto start = std::chrono::high_resolution_clock::now();

                intersection = Intersect(QTries, false, parallel);
                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                auto time = elapsed.count();
                total_time += time;
                part_time += time;
                if (i !=rep-1)
                    intersection.clear();
            }
            size_all_intersections += intersection.size();
            intersection.clear();

            ++nq;
        }
    }
    for (auto T: tries)
        delete T.second;

    // std::cout << "sets_read,total_elements,avg_size_bits_per_element,number_queries,avg_time" << n_il << std::endl;
    std::cout << n_il << "," << total_elements << "," << (float)(total_size*8)/total_elements << "," << nq << "," << (double)(total_time*1e-3)/(nq*rep) << std::endl;
}

int main(int argc, char const *argv[]) {
    int mandatory = 2;
    std::string input_filename = "";
    std::string querylog_filename = "";
    int rank = 0;
    uint64_t min_size = 0;
    uint64_t max_size = -1;

    // (*) mandatory parameters
    if (argc < mandatory){
        std::cout << "Usage: " << argv[0] << " <collection_filename> <query_log_filename>" << std::endl;
        std::cout << "Optional parameters:" << std::endl;
        std::cout << "[--verbose] [--parallel] [--min_size m] [--max_size m] [--rank v/v5/il] [--wsize 8/16/32/64]" << std::endl;
        return 1;
    }
    
    input_filename = std::string(argv[1]);
    querylog_filename = std::string(argv[2]);

    for (int i = 3; i < argc; ++i){
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
        if (std::string(argv[i]) == "--min_size") {
            ++i;
            min_size = std::stoull(argv[i]);
        }
        if (std::string(argv[i]) == "--max_size") {
            ++i;
            max_size = std::stoull(argv[i]);
        }
        if (std::string(argv[i]) == "--rank") {
            ++i;
            if (std::string(argv[i]) == "v") {
                rank = 0;
            }
            else if (std::string(argv[i]) == "il") {
                rank = 1;
            }
            else {
                rank = 2; // Asumiendo v5 o cualquier otro
            }
        }
    }

    processCollectionAndQueries<512>(input_filename, querylog_filename, rank, min_size, max_size);

    return 0;
}