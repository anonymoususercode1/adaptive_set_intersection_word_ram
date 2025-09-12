#ifndef XRFAST_BINARY_TRIE_3W
#define XRFAST_BINARY_TRIE_3W

#include <iostream>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support_v.hpp>
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
class x3WRBinaryTrie{
    private:
        uint16_t height; // original height of trie
        uint16_t height_u;
        
        sdsl::bit_vector bTrie;
        rankType b_rank;        
        //  low levels
        vector<wordType> lastLevelHigh;
        vector<wordType> lastLevelMedium;
        vector<wordType> lastLevelLow;
        // rank at the last Level Low
        uint32_t* r_block; 
        uint32_t* r_block_m;
        uint8_t n_part_rank;

        inline uint32_t popcount(wordType value) {
            if (sizeof(wordType) == sizeof(uint64_t)) {
                return __builtin_popcountll(value);
            } else {
                return __builtin_popcount(static_cast<uint32_t>(value));
            }
        }
    public:
        x3WRBinaryTrie() = default;
        ~x3WRBinaryTrie() = default;

        x3WRBinaryTrie(vector<uint64_t> &set, uint64_t u, uint8_t n_part_rank) {
            x3WRBinaryTrie::r_block = nullptr;
            x3WRBinaryTrie::n_part_rank = n_part_rank;
            uint32_t n = set.size();
            x3WRBinaryTrie::height_u = floor(log2(u - 1)) +  1;
            x3WRBinaryTrie::height = height_u - log2(pow(sizeof(wordType)*8, 3)) + 1; //only works for height > 6
            uint64_t max_nodes     = 2 * (pow(2, height) - 1); 
            max_nodes = max_nodes > MAX_BIT ? MAX_BIT : max_nodes; // utils: MAX_BIT 100000000000
            x3WRBinaryTrie::bTrie = bit_vector(max_nodes, 0);

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

            uint64_t word_bits = sizeof(wordType)*8;
            uint64_t num_mask_bits = static_cast<uint64_t>(log2(pow(word_bits, 3)));

            while (!q.empty()) {
                count_nodes++; // count node visited
                // Get node to write
                tuple<uint64_t, uint64_t, uint64_t> s = q.front(); 
                uint64_t l, r, n;
                std::tie(l, r, n) = s;
                q.pop();

                if (level == height-1){
                    
                    wordType wordAux[word_bits]={0};

                    wordType wordH = 0; // For the High level word
                    wordType wordM = 0; // For the Medium level word
                    wordType wordL = 0; // For the Low level word

                    bool is_first_high = true; // To handle the first High word
                    uint64_t curr_index_high = 0; // For the High level index
                    bool is_first_medium = true; // To handle the first Medium word
                    uint64_t curr_index_medium = 0; // For the Medium level index

                    for (uint64_t i = l; i < r+1; ++i){

                        uint64_t element = set[i] & (((uint64_t)1 << num_mask_bits) - 1);
                                                
                        uint64_t index_high = element / (word_bits * word_bits);
                        uint64_t index_medium = (element / word_bits) % word_bits;
                        uint64_t index_low = element % word_bits; 
                        
                        
                        wordH |= ((wordType)1 << index_high); // Accumulate the bit in the High word

                        if (is_first_high) 
                        { 
                            is_first_high = false; 
                            curr_index_high = index_high; 
                        } 
                        if (curr_index_high != index_high) 
                        { 
                            curr_index_high = index_high;
                            x3WRBinaryTrie::lastLevelMedium.push_back(wordM); 
                            wordM = 0; // Reset the Medium word for the new High 
                            is_first_medium = true; // Reset the indicator for Medium 
                            x3WRBinaryTrie::lastLevelLow.push_back(wordL); 
                            
                            wordL = 0; // Reset the word Low for the new Medium 
                        } 

                        wordM |= ((wordType)1 << index_medium); // Accumulate the bit in the word Medium 

                        if (is_first_medium) 
                        { 
                            is_first_medium = false; 
                            curr_index_medium = index_medium; 
                        }
                        if (curr_index_medium != index_medium)
                        {
                            curr_index_medium = index_medium;
                            x3WRBinaryTrie::lastLevelLow.push_back(wordL);
                            wordL = 0; // Reset the Low word to the new Medium
                        }

                        wordL |= ((wordType)1 << index_low); // Accumulate the bit in the Low word

                    }


                    if (wordM != 0) {
                        x3WRBinaryTrie::lastLevelMedium.push_back(wordM);
                        wordM = 0;
                    }
                    if (wordL != 0) {
                        x3WRBinaryTrie::lastLevelLow.push_back(wordL);
                        wordL = 0;
                    }

                    x3WRBinaryTrie::lastLevelHigh.push_back(wordH);

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
                if (level == x3WRBinaryTrie::height) break;
            }

            buildRankLevelHigh();
            buildRankLevelMedium();
            x3WRBinaryTrie::bTrie.resize(index);
            x3WRBinaryTrie::b_rank = rankType(&bTrie);
        };

        // Build the trie with height log2(max(set)-1)+1
        x3WRBinaryTrie(vector<uint64_t> set){
            uint8_t n_part_rank = 2;
            uint64_t u = set.back();
            x3WRBinaryTrie(set, u, n_part_rank);
        };


        inline uint16_t getHeight(){
            return x3WRBinaryTrie::height;
        };

        inline uint64_t getNode(uint64_t &node_id, uint16_t level) {
            return ((bTrie[2 * node_id]) << 1) | bTrie[(2 * node_id)+1];
        };

        inline wordType getWordHigh(uint64_t &node_id){
            // high_pos = node_id - (x3WRBinaryTrie::bTrie.size()/2);
            return lastLevelHigh[node_id - (x3WRBinaryTrie::bTrie.size()/2)];
        };

        inline wordType getWordHigh(uint64_t &node_id, uint64_t &high_pos){
            high_pos = node_id - (x3WRBinaryTrie::bTrie.size()/2);
            return lastLevelHigh[high_pos];
        };

        inline uint64_t getWordHighPos(uint64_t &node_id){
            return node_id - (x3WRBinaryTrie::bTrie.size()/2);
        };

        inline wordType getWordMedium(uint64_t &node_id, uint64_t high_bit_pos, uint64_t &medium_pos){
            uint64_t high_pos = node_id - (x3WRBinaryTrie::bTrie.size()/2);
            uint32_t rank = rankLevelHigh(high_pos);
            medium_pos = rank + high_bit_pos;
            return lastLevelMedium[medium_pos];
        };

        inline wordType getWordMediumByHighPos(uint64_t high_pos, uint64_t high_bit_pos, uint64_t &medium_pos){
            uint32_t rank = rankLevelHigh(high_pos);
            medium_pos = rank+high_bit_pos;
            return lastLevelMedium[medium_pos];
        };

        inline wordType getWordLowByMediumPos(uint64_t medium_pos, uint64_t medium_bit_pos){
            uint32_t rank = rankLevelMedium(medium_pos);
            return lastLevelLow[rank+medium_bit_pos];
        };

        inline uint64_t getLeftChild(uint64_t &node_id, uint16_t level) {
            if (level >= getHeight() - 1)
                return 0;
            else 
                return x3WRBinaryTrie::b_rank((2*node_id)+1);
        };

        inline uint64_t getRightChild(uint64_t &node_id, uint16_t level) {
             if (level >= getHeight() - 1)
                return 0;
            else
                return x3WRBinaryTrie::b_rank((2*node_id)+2);
                
        };

        // return size of bytes of all data structure 
        inline uint64_t size_in_bytes() {

            uint64_t bv_size = sdsl::size_in_bytes(x3WRBinaryTrie::bTrie);
            uint64_t lastLHigh_size = sizeof(x3WRBinaryTrie::lastLevelHigh) + sizeof(wordType)*x3WRBinaryTrie::lastLevelHigh.size();
            uint64_t rankLLHigh_size = sizeof(uint32_t*) + sizeof(uint32_t)*(x3WRBinaryTrie::lastLevelHigh.size()+(n_part_rank-1))/n_part_rank;
            uint64_t lastLMedium_size = sizeof(x3WRBinaryTrie::lastLevelMedium) + sizeof(wordType)*x3WRBinaryTrie::lastLevelMedium.size();
            uint64_t rankLLMedium_size = sizeof(uint32_t*) + sizeof(uint32_t)*(x3WRBinaryTrie::lastLevelMedium.size()+(n_part_rank-1))/n_part_rank;
            uint64_t lastLLow_size = sizeof(x3WRBinaryTrie::lastLevelLow) + sizeof(wordType)*x3WRBinaryTrie::lastLevelLow.size();
            
            uint64_t rank_size = sdsl::size_in_bytes(x3WRBinaryTrie::b_rank);
            return bv_size +
                    rank_size +
                    lastLHigh_size +
                    rankLLHigh_size +
                    lastLMedium_size +
                    rankLLMedium_size +
                    lastLLow_size +
                    2 * sizeof(bool) +
                    sizeof(uint8_t) +
                    2 * sizeof(uint16_t);
        };

        uint64_t serialize(std::ostream &out) {
            out.write(reinterpret_cast<char*>(&height), sizeof(height));
            out.write(reinterpret_cast<char*>(&(x3WRBinaryTrie::n_part_rank)), sizeof(x3WRBinaryTrie::n_part_rank));

            uint64_t bvs_size, rank_size;
            bvs_size  = bTrie.serialize(out);
            rank_size = b_rank.serialize(out);
            
            uint64_t size_last_level_high = x3WRBinaryTrie::lastLevelHigh.size();
            out.write(reinterpret_cast<char*>(&size_last_level_high), sizeof(uint64_t));
            for (wordType x: x3WRBinaryTrie::lastLevelHigh)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));

            uint64_t size_last_level_medium = x3WRBinaryTrie::lastLevelMedium.size();
            out.write(reinterpret_cast<char*>(&size_last_level_medium), sizeof(uint64_t));
            for (wordType x: x3WRBinaryTrie::lastLevelMedium)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));

            uint64_t size_last_level_low = x3WRBinaryTrie::lastLevelLow.size();
            out.write(reinterpret_cast<char*>(&size_last_level_low), sizeof(uint64_t));
            for (wordType x: x3WRBinaryTrie::lastLevelLow)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));
                
            uint64_t r_block_size = (size_last_level_high + (n_part_rank - 1)) / n_part_rank;
            out.write(reinterpret_cast<char*>(&r_block_size), sizeof(uint64_t));
            for (size_t i = 0; i < r_block_size; i++)
                out.write(reinterpret_cast<char*>(&x3WRBinaryTrie::r_block[i]), sizeof(uint32_t));

            uint64_t r_block_m_size = (size_last_level_medium + (n_part_rank - 1)) / n_part_rank;
            out.write(reinterpret_cast<char*>(&r_block_m_size), sizeof(uint64_t));
            for (size_t i = 0; i < r_block_m_size; i++)
                out.write(reinterpret_cast<char*>(&x3WRBinaryTrie::r_block_m[i]), sizeof(uint32_t));
            

            return bvs_size + rank_size + 
                   sizeof(x3WRBinaryTrie::lastLevelHigh) + 
                   sizeof(x3WRBinaryTrie::lastLevelMedium) + 
                   sizeof(x3WRBinaryTrie::lastLevelLow) + 
                   sizeof(x3WRBinaryTrie::r_block) + 
                   sizeof(x3WRBinaryTrie::r_block_m) + 
                   sizeof(n_part_rank) +
                   sizeof(height);
        }
        
        // load structure from in "in" stream
        void load(std::istream &in){
            in.read(reinterpret_cast<char*>(&height), sizeof(height));
            in.read(reinterpret_cast<char*>(&(x3WRBinaryTrie::n_part_rank)), sizeof(x3WRBinaryTrie::n_part_rank));

            x3WRBinaryTrie::bTrie.load(in);
            x3WRBinaryTrie::b_rank.load(in, &bTrie);

            height_u = height + (uint16_t)(log2(pow(sizeof(wordType)*8,2))) - 1;
            
            uint64_t size_last_level_high;
            in.read(reinterpret_cast<char*>(&size_last_level_high), sizeof(uint64_t));
            x3WRBinaryTrie::lastLevelHigh.reserve(size_last_level_high);
            for (uint64_t i = 0; i < size_last_level_high; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x3WRBinaryTrie::lastLevelHigh.push_back(x);
            }
                        
            uint64_t size_last_level_medium;
            in.read(reinterpret_cast<char*>(&size_last_level_medium), sizeof(uint64_t));
            x3WRBinaryTrie::lastLevelMedium.reserve(size_last_level_medium);
            for (uint64_t i = 0; i < size_last_level_medium; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x3WRBinaryTrie::lastLevelMedium.push_back(x);
            }

            uint64_t size_last_level_low;
            in.read(reinterpret_cast<char*>(&size_last_level_low), sizeof(uint64_t));
            x3WRBinaryTrie::lastLevelLow.reserve(size_last_level_low); 
            for (uint64_t i = 0; i < size_last_level_low; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x3WRBinaryTrie::lastLevelLow.push_back(x);
            }
            
            uint64_t r_block_size;
            in.read(reinterpret_cast<char*>(&r_block_size), sizeof(uint64_t));
            x3WRBinaryTrie::r_block = new uint32_t[r_block_size]();
            for (uint64_t i = 0; i < r_block_size; ++i){
                uint32_t x;
                in.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
                x3WRBinaryTrie::r_block[i] = x;
            }
            uint64_t r_block_m_size;
            in.read(reinterpret_cast<char*>(&r_block_m_size), sizeof(uint64_t));
            x3WRBinaryTrie::r_block_m = new uint32_t[r_block_m_size]();
            for (uint64_t i = 0; i < r_block_m_size; ++i){
                uint32_t x;
                in.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
                x3WRBinaryTrie::r_block_m[i] = x;
            }
        };

        
        inline void recursiveDecode(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level, uint64_t &word_bits) {

            if (curr_level == x3WRBinaryTrie::height-1) {
                uint64_t pos_high;
                // wordType w = getWordHigh(node_id, pos_high);
                wordType w = getWordHigh(node_id);
                wordType wordHigh = w;
                while (w != 0){
                    wordType t = w & (~w + 1);            
                    uint64_t z = __builtin_ctzll(w);
                    // //  AND at the medium part
                    // wordType wM = (~0);
                    
                    // Create a mask with bits set up to the given position
                    uint64_t mask = z == 63 ? (~0) : ((uint64_t)1 << (z + 1)) - 1;
                    // Apply the mask to the number
                    uint64_t masked_number = (uint64_t)wordHigh & mask;
                    //Count the number of bits set to 1
                    uint64_t pos = __builtin_popcountll(masked_number) -1;
                    
                    uint64_t pos_medium;
                    // wordType wM = getWordMediumByHighPos(pos_high, pos, pos_medium);  
                    wordType wM = getWordMedium(node_id, pos, pos_medium); 
                    wordType wordMedium = wM;

                    while (wM != 0){
                        wordType tMedium = wM & (~wM + 1);            
                        uint64_t zMedium = __builtin_ctzll(wM);

                        //  AND at the low part
                        wordType wLow = (~0);                    
                        // Create a mask with bits set up to the given position
                        uint64_t maskM = zMedium == 63 ? (~0) : ((uint64_t)1 << (zMedium + 1)) - 1;
                        // Apply the mask to the number
                        uint64_t masked_number_m = (uint64_t)wordMedium & maskM;
                        //Count the number of bits set to 1
                        uint64_t posM = __builtin_popcountll(masked_number_m) -1;
                    
                        wLow &= getWordLowByMediumPos(pos_medium, posM);

                        while (wLow != 0){
                            wordType tLow = wLow & (~wLow + 1);            
                            uint64_t zLow = __builtin_ctzll(wLow);

                            decoded.push_back((z * word_bits * word_bits + zMedium * word_bits + zLow) + partial_int);
                            
                            wLow ^= tLow;
                        }


                        
                        wM ^= tMedium;
                    }
                    w ^= t;
                }
                return;

            }

            uint64_t node = getNode(node_id, curr_level);
            uint64_t leftResult = partial_int;
            uint64_t rightResult = partial_int;

            if (node == 0b10 || node == 0b11) {
                recursiveDecode(decoded, leftResult, getLeftChild(node_id, curr_level), curr_level+1, word_bits);
            }
            if (node == 0b01 || node == 0b11) {
                rightResult = (rightResult | ((uint64_t)1 << (height_u - curr_level - 1)));
                recursiveDecode(decoded, rightResult, getRightChild(node_id, curr_level), curr_level+1, word_bits);
            }
        };

        inline void decode( vector<uint64_t> &decoded) {
            uint64_t partial_int = 0x00;            
            uint64_t word_bits = sizeof(wordType)*8;
            recursiveDecode(decoded, partial_int, 0, 0, word_bits);
        };

        
        // rank at the last Level Medium
        inline void buildRankLevelHigh() {
            
            uint32_t curr_word = 0, count = 0;
            // if (x3WRBinaryTrie::r_block != nullptr) {
            //     delete[] x3WRBinaryTrie::r_block;
            //     x3WRBinaryTrie::r_block = nullptr;
            // }
            uint32_t size_level = x3WRBinaryTrie::lastLevelHigh.size();
            x3WRBinaryTrie::r_block = new uint32_t[(size_level + (n_part_rank-1)) / n_part_rank]();
            for (size_t i = 0; i < size_level; i++)
            {
                if (i % n_part_rank == 0)
                    x3WRBinaryTrie::r_block[curr_word++] = count;
                count += popcount(x3WRBinaryTrie::lastLevelHigh[i]);
            }
        }

        // rank at the last Level Low
        inline void buildRankLevelMedium() {
            
            uint32_t curr_word = 0, count = 0;
            // if (x3WRBinaryTrie::r_block_m != nullptr) {
            //     delete[] x3WRBinaryTrie::r_block_m;
            //     x3WRBinaryTrie::r_block_m = nullptr;
            // }
            uint32_t size_level = x3WRBinaryTrie::lastLevelMedium.size();
            x3WRBinaryTrie::r_block_m = new uint32_t[(size_level + (n_part_rank-1)) / n_part_rank]();
            for (size_t i = 0; i < size_level; i++)
            {
                if (i % n_part_rank == 0)
                    x3WRBinaryTrie::r_block_m[curr_word++] = count;
                count += popcount(x3WRBinaryTrie::lastLevelMedium[i]);
            }
        }

        inline uint32_t rankLevelHigh(uint32_t pos) {
            uint32_t r_pos = pos / n_part_rank; 
            uint32_t start_pos = r_pos * n_part_rank;
            uint32_t rank = x3WRBinaryTrie::r_block[r_pos];
            for (size_t i = start_pos; i < pos; i++)
            {   
                rank += popcount(x3WRBinaryTrie::lastLevelHigh[i]);
            }
            
            return rank;
        }

        inline uint32_t rankLevelMedium(uint32_t pos) {
            uint32_t r_pos = pos / n_part_rank; 
            uint32_t start_pos = r_pos * n_part_rank;
            uint32_t rank = x3WRBinaryTrie::r_block_m[r_pos];
            for (size_t i = start_pos; i < pos; i++)
            {   
                rank += popcount(x3WRBinaryTrie::lastLevelMedium[i]);
            }
            return rank;
        }

        // free memory of uint32_t * r_block
        inline void free(){
            delete[] x3WRBinaryTrie::r_block;
            x3WRBinaryTrie::r_block = nullptr;
            delete[] x3WRBinaryTrie::r_block_m;
            x3WRBinaryTrie::r_block_m = nullptr;
        }



        inline void print() {
            std::cout << "LevelHigh:" << std::endl;
            for (int i = 0; i < lastLevelHigh.size(); ++i )
            {
                std::cout << lastLevelHigh[i] << " " << std::endl;
            }
                        std::cout << "lastLevelMedium:" << std::endl;
            for (int i = 0; i < lastLevelMedium.size(); ++i )
            {
                std::cout << lastLevelMedium[i] << " " << std::endl;
            }
                        std::cout << "lastLevelLow:" << std::endl;
            for (int i = 0; i < lastLevelLow.size(); ++i )
            {
                std::cout << lastLevelLow[i] << " " << std::endl;
            }
            
            std::cout << "rank lastLevelHigh:" << std::endl;
            for (int i = 0; i < lastLevelHigh.size(); ++i )
            {
                std::cout << rankLevelHigh(i) << " " << std::endl;
            }
            
            std::cout << "rank lastLevelMedium:" << std::endl;
            for (int i = 0; i < lastLevelMedium.size(); ++i )
            {
                std::cout << rankLevelMedium(i) << " " << std::endl;
            }

            std::cout << "lastLevelHigh.size: " << lastLevelHigh.size()<< std::endl;
            std::cout << "lastLevelMedium.size: " << lastLevelMedium.size()<< std::endl;
            std::cout << "lastLevelLow.size: " << lastLevelLow.size()<< std::endl;
            uint64_t count = 0;
            for (int i = 0; i < lastLevelLow.size(); ++i )
            {
                count += popcount(x3WRBinaryTrie::lastLevelLow[i]);
            }
            std::cout << count << std::endl;

        }




    std::string to_binary_string(wordType value, int num_bits) {
        if (num_bits == 0) return "";
        std::string binary_str = "";
        for (int i = num_bits - 1; i >= 0; --i) {
            binary_str += ((value >> i) & 1) ? '1' : '0';
        }
        return binary_str;
    }
};

#endif