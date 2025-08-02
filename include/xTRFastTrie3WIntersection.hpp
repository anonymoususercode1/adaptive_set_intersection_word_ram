#ifndef XTRFAST_TRIE_3W_INTERSECTION
#define XTRFAST_TRIE_3W_INTERSECTION

#include <vector>
#include "xTRFastBinaryTrie3W.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<xTRFastBinaryTrie3W<rankType, wordType>*> &Ts, bool parallel, bool balance);

#endif