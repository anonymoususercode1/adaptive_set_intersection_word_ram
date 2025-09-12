#ifndef XRFAST_BINARY_TRIE
#define XRFAST_BINARY_TRIE

#include <iostream>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>
#include <sdsl/rank_support_v5.hpp>
#include <vector>
#include <queue>
#include <math.h>
#include <thread>
#include <bit>
#include <cstdint>
#include <bitset>
#include "utils.hpp"

using namespace sdsl;
using namespace std;


template <typename rankType, typename wordType>
class x2WRBinaryTrie{
    private:
        uint16_t height; // original height of trie
        uint16_t height_with_runs;
        uint16_t height_u;
        bool runs_encoded;
        bool empty_trie;
        sdsl::bit_vector bTrie;
        rankType b_rank;        
        //  low levels
        vector<wordType> lastLevelHigh;
        vector<wordType> lastLevelLow;
        // rank at the last Level Low
        uint32_t* r_block;
        uint8_t n_part_rank;

        inline uint32_t popcount(wordType value) {
            if (sizeof(wordType) == sizeof(uint64_t)) {
                return __builtin_popcountll(value);
            } else {
                return __builtin_popcount(static_cast<uint32_t>(value));
            }
        }
    public:
        x2WRBinaryTrie() = default;
        ~x2WRBinaryTrie() = default;

        x2WRBinaryTrie(vector<uint64_t> &set, uint64_t u, uint8_t n_part_rank) {
            x2WRBinaryTrie::runs_encoded = false;
            x2WRBinaryTrie::r_block = nullptr;
            x2WRBinaryTrie::n_part_rank = n_part_rank;
            uint32_t n = set.size();
            x2WRBinaryTrie::height_u = floor(log2(u - 1)) +  1;
            x2WRBinaryTrie::height = height_u - log2(pow(sizeof(wordType)*8,2)) + 1; //only works for height > 6
            uint64_t max_nodes     = 2 * (pow(2, height) - 1); 
            max_nodes = max_nodes > MAX_BIT ? MAX_BIT : max_nodes; // utils: MAX_BIT 100000000000
            x2WRBinaryTrie::bTrie = bit_vector(max_nodes, 0);

            queue<tuple<uint64_t, uint64_t, uint64_t>> q;         
            // add all set to split
            tuple<uint64_t, uint64_t, uint64_t> split(0, n-1, n);
            q.push(split);

            uint16_t level            = 0;
            uint64_t nodes_curr_level = 1; 
            uint64_t count_nodes      = 0;
            uint64_t nodes_next_level = 0;
            uint64_t index            = 0;
            uint64_t total_nodes      = 0;
            uint64_t nodes_last_level = 0;

            while (!q.empty()) {
                count_nodes++; // count node visited
                // Get node to write
                tuple<uint64_t, uint64_t, uint64_t> s = q.front(); 
                uint64_t l, r, n;
                std::tie(l, r, n) = s;
                q.pop();

                if (level == height-1){

                    uint64_t word_bits = sizeof(wordType)*8;
                    wordType wordAux[word_bits]={0};

                    for (uint64_t i = l; i < r+1; ++i){
                        
                        uint64_t element = set[i] & (((uint64_t)1 << (uint64_t)(log2(pow(word_bits,2)))) - 1);
                        
                        uint64_t byteIndex = element / word_bits;    // Byte index in the vector
                        uint64_t bitOffset = element % word_bits;    // Bit offset in byte
                        wordAux[byteIndex] |= ((wordType)1 << bitOffset);  //Set the bit                          

                    }

                    wordType wordH = 0;

                    for (uint64_t i = 0; i < word_bits; i++)
                    {
                        if (wordAux[i] != 0)
                        {
                            x2WRBinaryTrie::lastLevelLow.push_back(wordAux[i]);
                            wordH |= ((wordType)1 << i); 
                        }                        
                    }
                    x2WRBinaryTrie::lastLevelHigh.push_back(wordH);

                    if (count_nodes == nodes_curr_level) break;
                }
                else {
                    uint64_t left_elements  = 0;
                    uint64_t right_elements = 0;

                    // j-th most significative bit
                    uint8_t j = height_u - level;
                    uint64_t ll, lr, rl, rr;
                    for (uint64_t i = l; i < r+1; ++i) {
                        if ((set[i] >> (j-1)) & 1) {                        
                            right_elements = r-i+1;
                            rl = i;
                            rr = r;
                            break;
                        }
                        else {
                            if (i == l){
                                ll = l;
                            }
                            lr = i;    
                            left_elements++;
                        }
                    }
                    // Add to queue split sets and write nodes
                    tuple<uint64_t,uint64_t,uint64_t> left_split;
                    tuple<uint64_t,uint64_t,uint64_t> right_split;
                    // left child
                    if (left_elements > 0) {
                        // write 1
                        bTrie[index] = 1;

                        tuple<uint64_t,uint64_t,uint64_t> left_split(ll, lr, left_elements);
                        q.push(left_split);
                        nodes_next_level++;
                        index++;
                        total_nodes++;
                    }
                    else {
                        // write 0
                        index++;
                        total_nodes++;
                    }
                    // right child
                    if (right_elements > 0) {
                        // write 1
                        bTrie[index] = 1;
                        
                        tuple<uint64_t,uint64_t,uint64_t> right_split(rl, rr, right_elements);
                        q.push(right_split);
                        nodes_next_level++;
                        index++;
                        total_nodes++;
                    }
                    else {
                        // write 0
                        index++;
                        total_nodes++;
                    }

                    if (count_nodes == nodes_curr_level) {
                        if (level == height-2){
                            nodes_last_level = nodes_next_level;
                            // index = 0;
                        }
                        nodes_curr_level = nodes_next_level;
                        nodes_next_level = 0;
                        count_nodes = 0;
                        level++;
                    }
                }
                if (level == x2WRBinaryTrie::height) break;
            }

            buildRankLevelHigh();
            x2WRBinaryTrie::bTrie.resize(index);
            x2WRBinaryTrie::b_rank = rankType(&bTrie);
        };

        // Build the trie with height log2(max(set)-1)+1
        x2WRBinaryTrie(vector<uint64_t> set){
            uint8_t n_part_rank = 2;
            uint64_t u = set.back();
            x2WRBinaryTrie(set, u, n_part_rank);
        };


        inline uint16_t getHeight(){
            return x2WRBinaryTrie::height;
        };

        inline uint64_t getNode(uint64_t &node_id, uint16_t level) {
            return ((bTrie[2 * node_id]) << 1) | bTrie[(2 * node_id)+1];
        };

        inline wordType getWordHigh(uint64_t &node_id){
            return lastLevelHigh[node_id - (x2WRBinaryTrie::bTrie.size()/2)];
        };

        inline wordType getWordHighPos(uint64_t &node_id){
            return node_id - (x2WRBinaryTrie::bTrie.size()/2);
        };

        inline wordType getWordLow(uint64_t &node_id, uint64_t high_bit_pos){
            uint64_t high_pos = node_id - (x2WRBinaryTrie::bTrie.size()/2);
            uint32_t rank = rankLevelHigh(high_pos);
            return lastLevelLow[rank+high_bit_pos];
        };
        
        inline uint64_t getWordLowPos(uint64_t &node_id, uint64_t high_bit_pos){
            uint64_t high_pos = node_id - (x2WRBinaryTrie::bTrie.size()/2);
            uint32_t rank = rankLevelHigh(high_pos);
            return rank+high_bit_pos;
        };

        inline wordType getWordLowByHighPos(uint64_t high_pos, uint64_t high_bit_pos){
            uint32_t rank = rankLevelHigh(high_pos);
            return lastLevelLow[rank+high_bit_pos];
        };

        inline wordType getWordLowByPos(uint64_t pos){
            return lastLevelLow[pos];
        };

        inline uint64_t getLeftChild(uint64_t &node_id, uint16_t level) {
            if (level >= getHeight() - 1)
                return 0;
            else 
                return x2WRBinaryTrie::b_rank((2*node_id)+1);
        };

        inline uint64_t getRightChild(uint64_t &node_id, uint16_t level) {
             if (level >= getHeight() - 1)
                return 0;
            else
                return x2WRBinaryTrie::b_rank((2*node_id)+2);
                
        };

        // return size of bytes of all data structure  
        inline uint64_t size_in_bytes() {

            uint64_t bv_size = sdsl::size_in_bytes(x2WRBinaryTrie::bTrie);
            uint64_t lastLHigh_size = sizeof(x2WRBinaryTrie::lastLevelHigh) + sizeof(wordType)*x2WRBinaryTrie::lastLevelHigh.size();
            uint64_t rankLLHigh_size = sizeof(uint32_t*) + sizeof(uint32_t)*(x2WRBinaryTrie::lastLevelHigh.size()+(n_part_rank-1))/n_part_rank;
            uint64_t lastLLow_size = sizeof(x2WRBinaryTrie::lastLevelLow) + sizeof(wordType)*x2WRBinaryTrie::lastLevelLow.size();
            
            uint64_t rank_size = sdsl::size_in_bytes(x2WRBinaryTrie::b_rank);
            return bv_size +
                    rank_size +
                    lastLHigh_size +
                    rankLLHigh_size +
                    lastLLow_size +
                    2 * sizeof(bool) +
                    sizeof(uint8_t) +
                    2 * sizeof(uint16_t);
        };

        uint64_t serialize(std::ostream &out) {
            out.write(reinterpret_cast<char*>(&height)          , sizeof(height));
            out.write(reinterpret_cast<char*>(&(x2WRBinaryTrie::height_with_runs)), sizeof(x2WRBinaryTrie::height_with_runs));
            out.write(reinterpret_cast<char*>(&(x2WRBinaryTrie::empty_trie))      , sizeof(x2WRBinaryTrie::empty_trie));
            out.write(reinterpret_cast<char*>(&(x2WRBinaryTrie::runs_encoded))    , sizeof(x2WRBinaryTrie::runs_encoded));
            out.write(reinterpret_cast<char*>(&(x2WRBinaryTrie::n_part_rank))    , sizeof(x2WRBinaryTrie::n_part_rank));

            uint64_t bvs_size, rank_size;
            bvs_size  = bTrie.serialize(out);
            rank_size = b_rank.serialize(out);
            
            uint64_t size_last_level_high = x2WRBinaryTrie::lastLevelHigh.size();
            out.write(reinterpret_cast<char*>(&size_last_level_high), sizeof(uint64_t));
            for (wordType x: x2WRBinaryTrie::lastLevelHigh)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));

            uint64_t size_last_level_low = x2WRBinaryTrie::lastLevelLow.size();
            out.write(reinterpret_cast<char*>(&size_last_level_low), sizeof(uint64_t));
            for (wordType x: x2WRBinaryTrie::lastLevelLow)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));
                
            uint64_t r_block_size = (size_last_level_high+(n_part_rank-1))/n_part_rank;
            out.write(reinterpret_cast<char*>(&r_block_size), sizeof(uint64_t));
            for (size_t i = 0; i < r_block_size; i++)
                out.write(reinterpret_cast<char*>(&x2WRBinaryTrie::r_block[i]), sizeof(uint32_t));
            

            return bvs_size + rank_size + 
                   sizeof(x2WRBinaryTrie::lastLevelHigh) + 
                   sizeof(x2WRBinaryTrie::lastLevelLow) + 
                   sizeof(x2WRBinaryTrie::r_block) + 
                   sizeof(n_part_rank) +
                   sizeof(height) + sizeof(height_with_runs) +
                   sizeof(empty_trie) + sizeof(runs_encoded);
        }
        
        // load structure from in "in" stream
        void load(std::istream &in){
            in.read(reinterpret_cast<char*>(&height)          , sizeof(height));
            in.read(reinterpret_cast<char*>(&(x2WRBinaryTrie::height_with_runs)), sizeof(x2WRBinaryTrie::height_with_runs));
            in.read(reinterpret_cast<char*>(&(x2WRBinaryTrie::empty_trie))      , sizeof(x2WRBinaryTrie::empty_trie));
            in.read(reinterpret_cast<char*>(&(x2WRBinaryTrie::runs_encoded))    , sizeof(x2WRBinaryTrie::runs_encoded));
            in.read(reinterpret_cast<char*>(&(x2WRBinaryTrie::n_part_rank))    , sizeof(x2WRBinaryTrie::n_part_rank));

            x2WRBinaryTrie::bTrie.load(in);
            b_rank.load(in, &bTrie);

            height_u = height + (uint16_t)(log2(pow(sizeof(wordType)*8,2))) - 1;
            
            uint64_t size_last_level_high;
            in.read(reinterpret_cast<char*>(&size_last_level_high), sizeof(uint64_t));
            x2WRBinaryTrie::lastLevelHigh.reserve(size_last_level_high);
            for (uint64_t i = 0; i < size_last_level_high; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x2WRBinaryTrie::lastLevelHigh.push_back(x);
            }

            uint64_t size_last_level_low;
            in.read(reinterpret_cast<char*>(&size_last_level_low), sizeof(uint64_t));
            x2WRBinaryTrie::lastLevelLow.reserve(size_last_level_low); 
            for (uint64_t i = 0; i < size_last_level_low; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x2WRBinaryTrie::lastLevelLow.push_back(x);
            }
            uint64_t r_block_size;
            in.read(reinterpret_cast<char*>(&r_block_size), sizeof(uint64_t));
            x2WRBinaryTrie::r_block = new uint32_t[r_block_size]();
            for (uint64_t i = 0; i < r_block_size; ++i){
                uint32_t x;
                in.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
                x2WRBinaryTrie::r_block[i] = x;
            }
        };


        void writeCompressTrie(vector<uint64_t> ones_to_write[], //vector array
                                vector<wordType> &newLastLevelHigh,
                                vector<wordType> &newLastLevelLow,
                                uint64_t* level_pos, 
                                uint16_t curr_level, uint64_t node_id, bool &its11){
            // End condition
            if (curr_level == (x2WRBinaryTrie::height-1)) {
                wordType w = getWordHigh(node_id);                
                newLastLevelHigh.push_back(w);

                if (w == ((wordType)~0))
                    its11 = true;                
                uint32_t w_pos = getWordHighPos(node_id), w_ones = popcount(w);
                uint32_t wl_pos_start = rankLevelHigh(w_pos);
                uint32_t wl_pos_end = wl_pos_start + w_ones;
                for (uint32_t i = wl_pos_start; i < wl_pos_end; ++i) {
                    uint64_t w_low = getWordLowByPos(i);
                    if( w_low != ((wordType)~0))
                        its11 = false;
                    newLastLevelLow.push_back(w_low);
                }

                ones_to_write[curr_level].push_back(level_pos[curr_level]);
                return;
            }

            uint64_t node = getNode(node_id, curr_level);
            uint16_t next_level = curr_level + 1;
            uint64_t next_level_pos = level_pos[next_level];
            bool its11_l = false;
            bool its11_r = false;

            if (node == 0b11) {
                uint64_t l_child = getLeftChild(node_id, curr_level);
                uint64_t r_child = l_child + 1;
                writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, next_level, l_child, its11_l);
                writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, next_level, r_child, its11_r);
                
                its11 = its11_l && its11_r;
                if (its11) {
                    if (curr_level == x2WRBinaryTrie::height - 2){
                        for (uint16_t i = 0; i < 2; ++i) {
                            wordType w = newLastLevelHigh.back();
                            uint64_t w_ones = popcount(w);
                            newLastLevelHigh.pop_back();
                            for(uint16_t k = 0; k < w_ones; ++k)
                                newLastLevelLow.pop_back();
                        }
                    }
                    // else 
                        level_pos[next_level] -= 4;
                }
                else {
                    ones_to_write[curr_level].push_back(level_pos[curr_level]);
                    ones_to_write[curr_level].push_back(level_pos[curr_level] + 1);      
                }
                level_pos[curr_level] += 2;
            }

            else if (node == 0b10 || node == 0b01){
                if (node == 0b10){
                    uint64_t l_child = getLeftChild(node_id, curr_level);
                    ones_to_write[curr_level].push_back(level_pos[curr_level]);
                    writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, next_level, l_child, its11_l);
                }
                level_pos[curr_level] += 1;
                if (node == 0b01) {
                    uint64_t r_child = getRightChild(node_id, curr_level);
                    ones_to_write[curr_level].push_back(level_pos[curr_level]);
                    writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, next_level, r_child, its11_r);
                }
                level_pos[curr_level] += 1;
            }
        };

        // Method write ones in a bit vector
        void writeOnes(vector<uint64_t> ones_to_write[], uint64_t* level_pos){
            uint64_t bits_n = 0;
            uint16_t last_level = 0;
            uint64_t bits_before_last_level;
            for (uint16_t level = 0; level < x2WRBinaryTrie::height-1; ++level) {
                bits_n += level_pos[level];
                if (level_pos[level] > 0) {
                    last_level = level;
                }              
            }

            x2WRBinaryTrie::height_with_runs = last_level + 1;
            x2WRBinaryTrie::bTrie = bit_vector(bits_n, 0);

            uint64_t global_level_pos = 0;
            for (uint16_t level = 0; level < x2WRBinaryTrie::height-1; ++level) {
                for (uint64_t i = 0; i < ones_to_write[level].size(); ++i) {
                    uint64_t pos = global_level_pos + ones_to_write[level][i];
                    x2WRBinaryTrie::bTrie[pos] = 1;
                }                global_level_pos += level_pos[level];
            }
            x2WRBinaryTrie::b_rank = rankType(&bTrie);
        };


        inline void encodeRuns() {
            vector<uint64_t> ones_to_write[x2WRBinaryTrie::height];
            uint64_t *level_pos = new uint64_t[x2WRBinaryTrie::height];
            for(uint64_t i = 0; i < x2WRBinaryTrie::height; ++i) level_pos[i] = 0;
            
            vector<wordType> newLastLevelHigh;
            vector<wordType> newLastLevelLow;
            bool itsOneOne = false;
            writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, 0, 0, itsOneOne);
            x2WRBinaryTrie::lastLevelHigh = newLastLevelHigh;
            x2WRBinaryTrie::lastLevelLow = newLastLevelLow;
            buildRankLevelHigh();
            writeOnes(ones_to_write, level_pos);
            x2WRBinaryTrie::runs_encoded = true;
            delete[] level_pos;
        };

        
        inline void recursiveDecode(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level) {
            
            if (curr_level == x2WRBinaryTrie::height-1) {
                wordType w = getWordHigh(node_id);

                while (w != 0){
                    wordType t = w & (~w + 1);            
                    uint64_t z = __builtin_ctzll(w);
                    //  AND at the low part
                    wordType wLow = (~0);
                    
                    // Create a mask with bits set up to the given position
                    uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                    // Apply the mask to the number
                    uint64_t masked_number = (uint64_t)getWordHigh(node_id) & mask;
                    //Count the number of bits set to 1
                    uint64_t pos = __builtin_popcountll(masked_number) -1;
                
                    wLow &= getWordLow(node_id, pos);
            

                    while (wLow != 0){
                        wordType tLow = wLow & (~wLow + 1);            
                        uint64_t zLow = __builtin_ctzll(wLow);
                        decoded.push_back((z * sizeof(wordType)*8 + zLow) + partial_int);
                        
                        wLow ^= tLow;
                    }
                    w ^= t;
                }
                return;

            }

            uint64_t node = getNode(node_id, curr_level);
            uint64_t leftResult = partial_int;
            uint64_t rightResult = partial_int;

            if (node == 0b10 || node == 0b11) {
                recursiveDecode(decoded, leftResult, getLeftChild(node_id, curr_level), curr_level+1);
            }
            if (node == 0b01 || node == 0b11) {
                rightResult = (rightResult | ((uint64_t)1 << (height_u - curr_level - 1)));
                recursiveDecode(decoded, rightResult, getRightChild(node_id, curr_level), curr_level+1);
            }
        };


        inline void runsRecursiveDecode(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level) {
            
            
            if (curr_level == x2WRBinaryTrie::height-1) {
                wordType w = getWordHigh(node_id);

                while (w != 0){
                    wordType t = w & (~w + 1);            
                    uint64_t z = __builtin_ctzll(w);
                    //  AND at the low part
                    wordType wLow = (~0);
                    
                    // Create a mask with bits set up to the given position
                    uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                    // Apply the mask to the number
                    uint64_t masked_number = (uint64_t)getWordHigh(node_id) & mask;
                    //Count the number of bits set to 1
                    uint64_t pos = __builtin_popcountll(masked_number) -1;
                
                    wLow &= getWordLow(node_id, pos);
            

                    while (wLow != 0){
                        wordType tLow = wLow & (~wLow + 1);            
                        uint64_t zLow = __builtin_ctzll(wLow);
                        decoded.push_back((z * sizeof(wordType)*8 + zLow) + partial_int);
                        
                        wLow ^= tLow;
                    }
                    w ^= t;
                }
                return;
            }

            uint64_t node = getNode(node_id, curr_level);

            if (node == 0b00) { 
                uint64_t below = partial_int;
                uint64_t range = ((uint64_t)1 << (height_u - curr_level)) - 1;
                uint64_t above = (partial_int | range);
                for (uint64_t i = below; i <= above; ++i) {
                    decoded.push_back(i);
                }
                return;
            }

            uint64_t leftResult  = partial_int;
            uint64_t rightResult = partial_int;

            if (node == 0b10 || node == 0b11) {
                runsRecursiveDecode(decoded, leftResult, getLeftChild(node_id, curr_level), curr_level+1);
            }
            if (node == 0b01 || node == 0b11) {
                rightResult = (rightResult | ((uint64_t)1 << (height_u - curr_level - 1)));
                runsRecursiveDecode(decoded, rightResult, getRightChild(node_id, curr_level), curr_level+1);
            }
        };


        inline void decode( vector<uint64_t> &decoded) {
            if (x2WRBinaryTrie::runs_encoded) {
                if (x2WRBinaryTrie::empty_trie) return;
                else {
                    uint64_t partial_int = 0x00;
                    runsRecursiveDecode(decoded, partial_int, 0, 0);
                }
            }
            else {
                uint64_t partial_int = 0x00;
                recursiveDecode(decoded, partial_int, 0, 0);
            }
        };

        
        // rank at the last Level Low
        inline void buildRankLevelHigh() {
            
            uint32_t curr_word = 0, count = 0;
            if (x2WRBinaryTrie::r_block != nullptr) {
                delete[] x2WRBinaryTrie::r_block;
                x2WRBinaryTrie::r_block = nullptr;
            }
            uint32_t size_level = x2WRBinaryTrie::lastLevelHigh.size();
            x2WRBinaryTrie::r_block = new uint32_t[(size_level+(n_part_rank-1))/n_part_rank]();
            for (size_t i = 0; i < size_level; i++)
            {
                if (i%n_part_rank == 0)
                    x2WRBinaryTrie::r_block[curr_word++] = count;
                count += popcount(x2WRBinaryTrie::lastLevelHigh[i]);
            }
        }

        inline uint32_t rankLevelHigh(uint32_t pos) {
            uint32_t r_pos = pos/n_part_rank; 
            uint32_t start_pos = r_pos*n_part_rank;
            uint32_t rank = x2WRBinaryTrie::r_block[r_pos];
            for (size_t i = start_pos; i < pos; i++)
            {   
                rank += popcount(x2WRBinaryTrie::lastLevelHigh[i]);
            }
            
            return rank;
        }

        // free memory of uint32_t * r_block
        inline void free(){
            delete[] x2WRBinaryTrie::r_block;
            x2WRBinaryTrie::r_block = nullptr;
        }



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        
        


        inline void recursiveDecodeTrieLevel(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level,  uint64_t upper_bits) {
            
            if (curr_level == x2WRBinaryTrie::height-1) {
                wordType w = getWordHigh(node_id);

                while (w != 0){
                    wordType t = w & (~w + 1);            
                    uint64_t z = __builtin_ctzll(w);
                    //  AND at the low part
                    wordType wLow = (~0);
                    
                    // Create a mask with bits set up to the given position
                    uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                    // Apply the mask to the number
                    uint64_t masked_number = (uint64_t)getWordHigh(node_id) & mask;
                    //Count the number of bits set to 1
                    uint64_t pos = __builtin_popcountll(masked_number) -1;
                
                    wLow &= getWordLow(node_id, pos);
            

                    while (wLow != 0){
                        wordType tLow = wLow & (~wLow + 1);            
                        uint64_t zLow = __builtin_ctzll(wLow);
                        uint64_t value_trie =  (z * sizeof(wordType)*8 + zLow) + partial_int;
                        // uint64_t value_orig = upper_bits | value_trie;
                        decoded.push_back(upper_bits | value_trie);
                        
                        wLow ^= tLow;
                    }
                    w ^= t;
                }
                return;

            }

            uint64_t node = getNode(node_id, curr_level);
            uint64_t leftResult = partial_int;
            uint64_t rightResult = partial_int;

            if (node == 0b10 || node == 0b11) {
                recursiveDecodeTrieLevel(decoded, leftResult, getLeftChild(node_id, curr_level), curr_level+1, upper_bits);
            }
            if (node == 0b01 || node == 0b11) {
                rightResult = (rightResult | ((uint64_t)1 << (height_u - curr_level - 1)));
                recursiveDecodeTrieLevel(decoded, rightResult, getRightChild(node_id, curr_level), curr_level+1, upper_bits);
            }
        };

        inline void runsRecursiveDecodeTrieLevel(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level,  uint64_t upper_bits) {
            
            
            if (curr_level == x2WRBinaryTrie::height-1) {
                wordType w = getWordHigh(node_id);

                while (w != 0){
                    wordType t = w & (~w + 1);            
                    uint64_t z = __builtin_ctzll(w);
                    //  AND at the low part
                    wordType wLow = (~0);
                    
                    // Create a mask with bits set up to the given position
                    uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                    // Apply the mask to the number
                    uint64_t masked_number = (uint64_t)getWordHigh(node_id) & mask;
                    //Count the number of bits set to 1
                    uint64_t pos = __builtin_popcountll(masked_number) -1;
                
                    wLow &= getWordLow(node_id, pos);
            

                    while (wLow != 0){
                        wordType tLow = wLow & (~wLow + 1);            
                        uint64_t zLow = __builtin_ctzll(wLow);
                        // decoded.push_back((z * sizeof(wordType)*8 + zLow) + partial_int);
                        uint64_t value_trie =  (z * sizeof(wordType)*8 + zLow) + partial_int;
                        // uint64_t value_orig = upper_bits | value_trie;
                        decoded.push_back( upper_bits | value_trie);
                        
                        wLow ^= tLow;
                    }
                    w ^= t;
                }
                return;
            }

            uint64_t node = getNode(node_id, curr_level);

            if (node == 0b00) { 
                uint64_t below = partial_int;
                uint64_t range = ((uint64_t)1 << (height_u - curr_level)) - 1;
                uint64_t above = (partial_int | range);
                for (uint64_t i = below; i <= above; ++i) {

                    decoded.push_back(upper_bits | i);
                }
                return;
            }

            uint64_t leftResult  = partial_int;
            uint64_t rightResult = partial_int;

            if (node == 0b10 || node == 0b11) {
                runsRecursiveDecodeTrieLevel(decoded, leftResult, getLeftChild(node_id, curr_level), curr_level+1, upper_bits);
            }
            if (node == 0b01 || node == 0b11) {
                rightResult = (rightResult | ((uint64_t)1 << (height_u - curr_level - 1)));
                runsRecursiveDecodeTrieLevel(decoded, rightResult, getRightChild(node_id, curr_level), curr_level+1, upper_bits);
            }
        };

        inline void decodeTrieLevel( vector<uint64_t> &decoded, uint64_t upper_bits ) {
            if (x2WRBinaryTrie::runs_encoded) {
                if (x2WRBinaryTrie::empty_trie) return;
                else {
                    uint64_t partial_int = 0x00;
                    runsRecursiveDecodeTrieLevel(decoded, partial_int, 0, 0, upper_bits);
                }
            }
            else {
                uint64_t partial_int = 0x00;
                recursiveDecodeTrieLevel(decoded, partial_int, 0, 0, upper_bits);
            }
        };


};

#endif