#ifndef FAST_TRIE_INTERSECTION
#define FAST_TRIE_INTERSECTION

#include <vector>
#include "wBinaryTrie.hpp"
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>

using namespace std;
using namespace sdsl;

template<typename rankType, typename wordType>
vector<uint64_t> Intersect(vector<wBinaryTrie<rankType, wordType>*> &Ts, bool runs, bool parallel);

// vector<uint64_t> forceBruteIntersection(vector<vector<uint64_t>> &S);
// template<>
// vector<uint64_t> Intersect<rank_support_v<1>>(vector<wBinaryTrie<rank_support_v<1>>*> &Ts, bool runs, bool parallel);
// template<>
// vector<uint64_t> Intersect<rank_support_v5<1>>(vector<wBinaryTrie<rank_support_v5<1>>*> &Ts, bool runs, bool parallel);


#endif