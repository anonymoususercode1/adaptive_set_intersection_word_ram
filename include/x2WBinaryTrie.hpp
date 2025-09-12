#ifndef XW_BINARY_TRIE
#define XW_BINARY_TRIE

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
#include <bitset>
#include "utils.hpp"

using namespace sdsl;
using namespace std;

template <typename rankType, typename wordType>
class x2WBinaryTrie{
    private:
        uint16_t height; // original height of trie
        uint16_t height_with_runs;
        uint16_t height_u;
        bool runs_encoded;
        bool empty_trie;
        sdsl::bit_vector bTrie;
        rankType b_rank;        

        vector<wordType> lastLevelHigh;
        vector<vector<wordType>> lastLevelLow;

        inline uint32_t popcount(wordType value) {
            if (sizeof(wordType) == sizeof(uint64_t)) {
                return __builtin_popcountll(value);
            } else {
                return __builtin_popcount(static_cast<uint32_t>(value));
            }
        }
    public:
        x2WBinaryTrie() = default;
        ~x2WBinaryTrie() = default;

        x2WBinaryTrie(vector<uint64_t> &set, uint64_t u) {
            x2WBinaryTrie::runs_encoded = false;
            uint32_t n = set.size();
            x2WBinaryTrie::height_u = floor(log2(u - 1)) +  1;
            x2WBinaryTrie::height = height_u - log2(pow(sizeof(wordType)*8,2)) + 1; //only works for height > 6
            uint64_t max_nodes     = 2 * (pow(2, height) - 1);
            max_nodes = max_nodes > MAX_BIT ? MAX_BIT : max_nodes; // utils: MAX_BIT 100000000000
            x2WBinaryTrie::bTrie = bit_vector(max_nodes, 0);

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
                    vector<wordType> wordLow;

                    for (uint64_t i = 0; i < word_bits; i++)
                    {
                        if (wordAux[i] != 0)
                        {
                            wordLow.push_back(wordAux[i]);
                            wordH |= ((wordType)1 << i); 
                        }                        
                    }

                    x2WBinaryTrie::lastLevelLow.push_back(wordLow);
                    x2WBinaryTrie::lastLevelHigh.push_back(wordH);

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
                if (level == x2WBinaryTrie::height) break;
            }

            x2WBinaryTrie::bTrie.resize(index);
            x2WBinaryTrie::b_rank = rankType(&bTrie);
        };

        // Build the trie with height log2(max(set)-1)+1
        x2WBinaryTrie(vector<uint64_t> set){
            uint64_t u = set.back();
            x2WBinaryTrie(set, u);
        };


        inline uint16_t getHeight(){
            return x2WBinaryTrie::height;
        };

        inline uint64_t getNode(uint64_t &node_id, uint16_t level) {
            return ((bTrie[2 * node_id]) << 1) | bTrie[(2 * node_id)+1];
        };

        inline wordType getWordHigh(uint64_t &node_id){
            return lastLevelHigh[node_id - (x2WBinaryTrie::bTrie.size()/2)];
        };        
        
        inline wordType getWordHighPos(uint64_t &node_id){
            return node_id - (x2WBinaryTrie::bTrie.size()/2);
        };

        inline wordType getWordLow(uint64_t &node_id, uint64_t low_pos){
            return lastLevelLow[node_id - (x2WBinaryTrie::bTrie.size()/2)][low_pos];
        };

        inline std::vector<wordType> getLastLevelLow(uint64_t &node_id){
            return lastLevelLow[node_id - (x2WBinaryTrie::bTrie.size()/2)];
        };

        inline uint64_t getLeftChild(uint64_t &node_id, uint16_t level) {
            if (level >= getHeight() - 1)
                return 0;
            else 
                return x2WBinaryTrie::b_rank((2*node_id)+1);
        };

        inline uint64_t getRightChild(uint64_t &node_id, uint16_t level) {
             if (level >= getHeight() - 1)
                return 0;
            else
                return x2WBinaryTrie::b_rank((2*node_id)+2);
                
        };

        // return size of bytes of all data structure
        inline uint64_t size_in_bytes() {

            uint64_t bv_size = sdsl::size_in_bytes(x2WBinaryTrie::bTrie);
            uint64_t lastL_size = sizeof(x2WBinaryTrie::lastLevelHigh) + 
                                  sizeof(wordType)*x2WBinaryTrie::lastLevelHigh.size();
            uint64_t lastLLow_size = sizeof(vector<vector<wordType>>) + 
                                     x2WBinaryTrie::lastLevelLow.size() * sizeof(vector<wordType>);

            for (uint64_t i = 0; i < x2WBinaryTrie::lastLevelHigh.size(); i++)
            {                
                lastLLow_size+=sizeof(wordType)*popcount(x2WBinaryTrie::lastLevelHigh[i]);
            }

            uint64_t rank_size = sdsl::size_in_bytes(x2WBinaryTrie::b_rank);
            return bv_size +
                    rank_size +
                    lastL_size +
                    lastLLow_size +
                    2 * sizeof(bool) +
                    2 * sizeof(uint8_t);
        };

        uint64_t serialize(std::ostream &out) {
            out.write(reinterpret_cast<char*>(&height)          , sizeof(height));
            out.write(reinterpret_cast<char*>(&(x2WBinaryTrie::height_with_runs)), sizeof(x2WBinaryTrie::height_with_runs));
            out.write(reinterpret_cast<char*>(&(x2WBinaryTrie::empty_trie))      , sizeof(x2WBinaryTrie::empty_trie));
            out.write(reinterpret_cast<char*>(&(x2WBinaryTrie::runs_encoded))    , sizeof(x2WBinaryTrie::runs_encoded));

            uint64_t bvs_size, rank_size;
            bvs_size  = bTrie.serialize(out);
            rank_size = b_rank.serialize(out);
            
            uint64_t size_last_level_high = lastLevelHigh.size();
            out.write(reinterpret_cast<char*>(&size_last_level_high), sizeof(uint64_t));
            for (wordType x: lastLevelHigh)
                out.write(reinterpret_cast<char*>(&x), sizeof(wordType));

            uint64_t size_last_level_low = lastLevelHigh.size();
            out.write(reinterpret_cast<char*>(&size_last_level_low), sizeof(uint64_t));
            for (vector<wordType> xPart: lastLevelLow){
                uint64_t size_low_part = xPart.size();
                out.write(reinterpret_cast<char*>(&size_low_part), sizeof(uint64_t));
                for (wordType part: xPart)
                    out.write(reinterpret_cast<char*>(&part), sizeof(wordType));
            }

            return bvs_size + rank_size + 
                   sizeof(lastLevelHigh) + sizeof(lastLevelLow) + 
                   sizeof(height) + sizeof(height_with_runs) +
                   sizeof(empty_trie) + sizeof(runs_encoded);
        }

        // load structure from in "in" stream
        void load(std::istream &in){
            in.read(reinterpret_cast<char*>(&height)          , sizeof(height));
            in.read(reinterpret_cast<char*>(&(x2WBinaryTrie::height_with_runs)), sizeof(x2WBinaryTrie::height_with_runs));
            in.read(reinterpret_cast<char*>(&(x2WBinaryTrie::empty_trie))      , sizeof(x2WBinaryTrie::empty_trie));
            in.read(reinterpret_cast<char*>(&(x2WBinaryTrie::runs_encoded))    , sizeof(x2WBinaryTrie::runs_encoded));

            x2WBinaryTrie::bTrie.load(in);
            b_rank.load(in, &bTrie);

            height_u = height + (uint16_t)(log2(pow(sizeof(wordType)*8,2))) - 1;
            
            uint64_t sizeLastLevelHigh;
            in.read(reinterpret_cast<char*>(&sizeLastLevelHigh), sizeof(uint64_t));
            x2WBinaryTrie::lastLevelHigh.reserve(sizeLastLevelHigh);
            for (uint64_t i = 0; i < sizeLastLevelHigh; ++i){
                wordType x;
                in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                x2WBinaryTrie::lastLevelHigh.push_back(x);
            }

            uint64_t sizeLastLevelLow;
            in.read(reinterpret_cast<char*>(&sizeLastLevelLow), sizeof(uint64_t));
            x2WBinaryTrie::lastLevelLow.reserve(sizeLastLevelLow); 
                
            for (uint64_t i = 0; i < sizeLastLevelLow; ++i){
                uint64_t sizeLowPart;
                in.read(reinterpret_cast<char*>(&sizeLowPart), sizeof(uint64_t));
                vector<wordType> lowPart;
                lowPart.reserve(sizeLowPart);
                for (uint64_t j = 0; j < sizeLowPart; ++j){
                    wordType x;
                    in.read(reinterpret_cast<char*>(&x), sizeof(wordType));
                    lowPart.push_back(x);
                }
                x2WBinaryTrie::lastLevelLow.push_back(lowPart);
            }
        };

        void writeCompressTrie(vector<uint64_t> ones_to_write[], //vector array
                                vector<wordType> &newLastLevelHigh,
                                vector<vector<wordType>> &newLastLevelLow,
                                uint64_t* level_pos, 
                                uint16_t curr_level, uint64_t node_id, bool &its11){
            // End condition
            if (curr_level == (x2WBinaryTrie::height-1)) {
                wordType w = getWordHigh(node_id);
                vector<wordType> wPart = getLastLevelLow(node_id);
                if (w == ((wordType)~0)) {
                    if(wPart.size() == sizeof(wordType)*8){
                        its11 = true;
                        for (uint16_t i = 0; i < wPart.size(); ++i)
                            if(wPart[i] != ((wordType)~0))
                                its11 = false;
                    }
                }
                newLastLevelHigh.push_back(w);
                newLastLevelLow.push_back(wPart);
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
                    if (curr_level == x2WBinaryTrie::height - 2){
                        for (uint16_t i = 0; i < 2; ++i) {
                            newLastLevelHigh.pop_back();
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
            for (uint16_t level = 0; level < x2WBinaryTrie::height-1; ++level) {
                bits_n += level_pos[level];
                if (level_pos[level] > 0) {
                    last_level = level;
                }              
            }

            x2WBinaryTrie::height_with_runs = last_level + 1;
            x2WBinaryTrie::bTrie = bit_vector(bits_n, 0);

            uint64_t global_level_pos = 0;
            for (uint16_t level = 0; level < x2WBinaryTrie::height-1; ++level) {
                for (uint64_t i = 0; i < ones_to_write[level].size(); ++i) {
                    uint64_t pos = global_level_pos + ones_to_write[level][i];
                    x2WBinaryTrie::bTrie[pos] = 1;
                }                global_level_pos += level_pos[level];
            }
            x2WBinaryTrie::b_rank = rankType(&bTrie);
        };


        inline void encodeRuns() {
            vector<uint64_t> ones_to_write[x2WBinaryTrie::height];
            uint64_t *level_pos = new uint64_t[x2WBinaryTrie::height];
            for(uint64_t i = 0; i < x2WBinaryTrie::height; ++i) level_pos[i] = 0;
            
            vector<wordType> newLastLevelHigh;
            vector<vector<wordType>> newLastLevelLow;
            bool itsOneOne = false;
            writeCompressTrie(ones_to_write, newLastLevelHigh, newLastLevelLow, level_pos, 0, 0, itsOneOne);
            x2WBinaryTrie::lastLevelHigh = newLastLevelHigh;
            x2WBinaryTrie::lastLevelLow = newLastLevelLow;
            
            writeOnes(ones_to_write, level_pos);
            x2WBinaryTrie::runs_encoded = true;
            delete[] level_pos;
        };

        
        inline void recursiveDecode(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level) {
            
            if (curr_level == x2WBinaryTrie::height-1) {
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
            
            if (curr_level == x2WBinaryTrie::height-1) {
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
            if (x2WBinaryTrie::runs_encoded) {
                if (x2WBinaryTrie::empty_trie) return;
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



        inline void recursiveDecodeTrieLevel(vector<uint64_t> &decoded, uint64_t partial_int, uint64_t node_id, uint16_t curr_level,  uint64_t upper_bits) {
            
            if (curr_level == x2WBinaryTrie::height-1) {
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
            
            
            if (curr_level == x2WBinaryTrie::height-1) {
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
            if (x2WBinaryTrie::runs_encoded) {
                if (x2WBinaryTrie::empty_trie) return;
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