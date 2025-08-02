
#ifndef XTFAST_BINARY_TRIE
#define XTFAST_BINARY_TRIE

#include <iostream>
#include <vector>
#include "xRFastBinaryTrie.hpp"

using namespace sdsl;
using namespace std;

template <typename rankType, typename wordType>
class xTRFastBinaryTrie{
    private:
        wordType firstLevel = 0;
        uint16_t height_u = 0;
        xRFastBinaryTrie<rankType, wordType>** trieLevel = nullptr;
        uint32_t* trieSize = nullptr;


        inline uint32_t popcount(wordType value) {
            if (sizeof(wordType) == sizeof(uint64_t)) {
                return __builtin_popcountll(value);
            } else {
                return __builtin_popcount(static_cast<uint32_t>(value));
            }
        }
    public:
        xTRFastBinaryTrie() = default;
        ~xTRFastBinaryTrie() {
            free(); // Free dynamic memory
        }

        // Build the trie with height log2(max(set)-1)+1
        xTRFastBinaryTrie(vector<uint64_t> set){
            uint8_t n_part_rank = 2;
            uint64_t u = set.back();
            xTRFastBinaryTrie(set, u, n_part_rank);
        };

        xTRFastBinaryTrie(vector<uint64_t>& set, uint64_t u, uint8_t n_part_rank) {
            xTRFastBinaryTrie::height_u = floor(log2(u - 1)) + 1;
            xTRFastBinaryTrie::firstLevel = 0;
            vector<uint64_t> currentSet;                 
            

            uint16_t firstBits = floor(log2(sizeof(wordType) * 8));
            uint16_t lastBits = height_u - firstBits;
            uint64_t new_u = pow(2, lastBits) - 1;
            uint64_t word_bits = sizeof(wordType) * 8;

            uint64_t currentFirst = -1;
            uint64_t first = -1;
            vector<uint32_t> trieSizeAux;

            vector<xRFastBinaryTrie<rankType, wordType>*> trieLevelAux;
            for (uint64_t value : set) {                
                // Get the firstBits most significant bits
                
                first = value >> lastBits;
                // Get the height_u-firstBits least significant bits
                uint64_t last = value & (((uint64_t)1 << lastBits) - 1);
                if (xTRFastBinaryTrie::firstLevel == 0)
                {
                    xTRFastBinaryTrie::firstLevel |= ((wordType)1 << first);
                } else if (!(xTRFastBinaryTrie::firstLevel & ((wordType)1 << first))) 
                {
                    xTRFastBinaryTrie::firstLevel |= ((wordType)1 << first);

                    vector<uint64_t> tempSet = std::move(currentSet); 
                    xRFastBinaryTrie<rankType, wordType>* trie = new xRFastBinaryTrie<rankType, wordType>(tempSet, new_u,n_part_rank);
                    trieLevelAux.push_back(trie);
                    trieSizeAux.push_back(tempSet.size());
                    
                    currentSet.clear(); 
                }
                
                currentSet.push_back(last);
            }

            if (currentSet.size() > 0) {                    
                
                vector<uint64_t> tempSet = std::move(currentSet); 
                xRFastBinaryTrie<rankType, wordType>* trie = new xRFastBinaryTrie<rankType, wordType>(tempSet, new_u, n_part_rank);
                trieLevelAux.push_back(trie);
                trieSizeAux.push_back(tempSet.size());
            }     
            free();
            trieSize = new uint32_t[trieSizeAux.size()];
            std::copy(trieSizeAux.begin(), trieSizeAux.end(), trieSize);  
 
            trieLevel  = new xRFastBinaryTrie<rankType, wordType>*[trieLevelAux.size()];
            
            for (size_t i = 0; i < trieLevelAux.size(); ++i) {
                trieLevel[i] = trieLevelAux[i];
            }
            trieLevelAux.clear();    
            
        }

        inline wordType getFirstLevel() const {
            return firstLevel;
        }

        inline xRFastBinaryTrie<rankType, wordType>** getTrieLevel() const {
            return trieLevel;
        }

        inline xRFastBinaryTrie<rankType, wordType>* getTrieLevelPos(uint64_t &pos) const {
            return trieLevel[pos];
        }

        inline uint32_t getTrieSizePos(uint64_t &pos) const {
            return trieSize[pos];
        }

        inline uint16_t getHeightU() const {
            return height_u;
        }

        inline uint16_t getTrieLevelSize() const {
            return popcount(xTRFastBinaryTrie::firstLevel);
        } 

        uint64_t serialize(std::ostream &out) {

            uint64_t bytesWritten = 0;            
            out.write(reinterpret_cast<char*>(&firstLevel), sizeof(wordType));
            bytesWritten += sizeof(wordType);            
            out.write(reinterpret_cast<char*>(&height_u), sizeof(uint16_t));
            bytesWritten += sizeof(uint16_t);
            
            uint16_t trieLevelSize = popcount(firstLevel);

            for (size_t i = 0; i < trieLevelSize; i++)
            {
                bytesWritten += trieLevel[i]->serialize(out); 
            }

            out.write(reinterpret_cast<char*>(trieSize), trieLevelSize * sizeof(uint32_t)); 
            bytesWritten += trieLevelSize * sizeof(uint32_t);

            return bytesWritten;  
        }

        void load(std::istream &in) {
            free();          
            in.read(reinterpret_cast<char*>(&firstLevel), sizeof(wordType));
            in.read(reinterpret_cast<char*>(&height_u), sizeof(uint16_t));
            
            uint16_t trieLevelSize = popcount(firstLevel);

           trieLevel = new xRFastBinaryTrie<rankType, wordType>*[trieLevelSize];
            for (uint16_t i = 0; i < trieLevelSize; ++i) {
                trieLevel[i] = new xRFastBinaryTrie<rankType, wordType>();
                trieLevel[i]->load(in);
            }

            trieSize = new uint32_t[trieLevelSize];
            in.read(reinterpret_cast<char*>(trieSize), trieLevelSize * sizeof(uint32_t));
        }

                // return size of bytes of all data structure
        inline uint64_t size_in_bytes() {
            uint64_t trieLevel_size = 0;
            uint16_t trieLevelSize = popcount(firstLevel);
            // Recorrer usando un índice
            for (uint16_t i = 0; i < trieLevelSize; ++i) {
                trieLevel_size += xTRFastBinaryTrie::trieLevel[i]->size_in_bytes() + sizeof(xTRFastBinaryTrie::trieLevel[i]);
            }

            return trieLevel_size + sizeof(wordType) + sizeof(uint16_t) +                   
                   sizeof(xTRFastBinaryTrie::trieLevel);
        }

        inline uint64_t size_all_in_bytes() {
            uint64_t trieLevel_size = 0;
            uint16_t trieLevelSize = popcount(firstLevel);
            // Recorrer usando un índice
            for (uint16_t i = 0; i < trieLevelSize; ++i) {
                trieLevel_size += xTRFastBinaryTrie::trieLevel[i]->size_in_bytes() + sizeof(xTRFastBinaryTrie::trieLevel[i]);
            }

            return trieLevel_size + sizeof(wordType) + sizeof(uint16_t) +                   
                   sizeof(xTRFastBinaryTrie::trieLevel) +  trieLevelSize * sizeof(uint32_t);
        }

        inline void decode( vector<uint64_t> &decoded) {

            wordType number = xTRFastBinaryTrie::firstLevel;
            uint64_t bits_needed = std::ceil(std::log2(sizeof(wordType) * 8));
            uint16_t pos = 0;
            uint64_t posOriginal = 0;
            uint64_t posOriginal2 = 0;
            while (number != 0) {
                // Encuentra su posición (número de ceros a la derecha)
                uint64_t bits_pos = __builtin_ctzll(number); 
                uint64_t upper_bits = bits_pos << (xTRFastBinaryTrie::height_u - bits_needed);
                
                vector<uint64_t> decodedAux;
                trieLevel[pos]->decodeTrieLevel(decodedAux, upper_bits);
    
                ++pos;
                decoded.insert(decoded.end(), decodedAux.begin(), decodedAux.end());
                
                // Encuentra el bit menos significativo activo
                uint64_t lowestBit = number & (~number + 1);
                // Apaga el bit menos significativo activo
                number ^= lowestBit;
            }


        };

        inline void free(){	

            if (trieLevel != nullptr) {
                uint16_t trieLevelSize = popcount(firstLevel);
                // Recorrer usando un índice
                for (uint16_t i = 0; i < trieLevelSize; ++i) {
                    if (trieLevel[i] != nullptr) {
                        delete trieLevel[i];
                        trieLevel[i] = nullptr;
                    }
                }
                delete[] trieLevel;
                trieLevel = nullptr;
            }
            
            if (trieSize != nullptr) {
                delete[] trieSize;
                trieSize = nullptr;
            }
        }
};

#endif
