#ifndef UTILS
#define UTILS


#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>



#define MAX_BIT 100000000000

struct Coordinate {
    uint32_t x;
    uint32_t y;
    uint32_t value;
};
struct Coordinate64 {
    uint64_t x;
    uint64_t y;
    uint64_t value;
};

uint32_t interleaveBits32(uint16_t x, uint16_t y);

uint16_t deinterleaveBitsX32(uint32_t z);

uint16_t deinterleaveBitsY32(uint32_t z);

uint64_t interleaveBits64(uint32_t x, uint32_t y);

uint32_t deinterleaveBitsX64(uint64_t z);

uint32_t deinterleaveBitsY64(uint64_t z);


#endif