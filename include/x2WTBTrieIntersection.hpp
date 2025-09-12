#ifndef XTFAST_TRIE_INTERSECTION
#define XTFAST_TRIE_INTERSECTION

#include <vector>
#include "x2WTBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<x2WTBinaryTrie<rankType, wordType>*> &Ts, bool parallel, bool balance);

#endif