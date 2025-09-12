#ifndef XRFAST_TRIE_3W_INTERSECTION
#define XRFAST_TRIE_3W_INTERSECTION

#include <vector>
#include "x3WRBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<x3WRBinaryTrie<rankType, wordType>*> &Ts, bool parallel);

#endif