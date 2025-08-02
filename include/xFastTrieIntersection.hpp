#ifndef XFAST_TRIE_INTERSECTION
#define XFAST_TRIE_INTERSECTION

#include <vector>
#include "xFastBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<xFastBinaryTrie<rankType, wordType>*> &Ts, bool runs, bool parallel);

#endif