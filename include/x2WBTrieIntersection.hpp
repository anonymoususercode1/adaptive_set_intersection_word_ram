#ifndef XW_TRIE_INTERSECTION
#define XW_TRIE_INTERSECTION

#include <vector>
#include "x2WBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<x2WBinaryTrie<rankType, wordType>*> &Ts, bool runs, bool parallel);

#endif