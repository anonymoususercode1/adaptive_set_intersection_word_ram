#ifndef XTRFAST_TRIE_INTERSECTION
#define XTRFAST_TRIE_INTERSECTION

#include <vector>
#include "x2WTRBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<x2WTRBinaryTrie<rankType, wordType>*> &Ts, bool parallel, bool balance);

#endif