#include "BitPattern.hpp"

int main(const int argc, const char *[]) {
    static_assert(0b1101010u == BitPattern("11XXX10")); 

//    constexpr BitPattern pattern("101XXX111000101XX0101");
//    return (static_cast<std::uint32_t>(argc) == pattern);
    return (static_cast<std::uint32_t>(argc) == BitPattern("101XXX111000101XX0101"));
}
