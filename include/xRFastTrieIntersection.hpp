#ifndef XRFAST_TRIE_INTERSECTION
#define XRFAST_TRIE_INTERSECTION

#include <vector>
#include "xRFastBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<xRFastBinaryTrie<rankType, wordType>*> &Ts, bool runs, bool parallel);

#endif