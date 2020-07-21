#include <cstdint>
#include <stdexcept>
#include <type_traits>

class BitPattern {
 private:
  // expected is the expected result after masking
  std::uint64_t expected{};

  // mask is set of bits I care about
  std::uint64_t mask{0xFFFFFFFFFFFFFFFF};

  // input & mask == expected

 public:
  template <std::size_t Size>
  explicit consteval BitPattern(const char (&input)[Size]) {
    std::uint64_t cur_bit = (1 << (Size - 2));
    for (const char val : input) {
      if (val == 0) {
        return;
      }

      if (val == '1') {
        expected |= cur_bit;
      } else if (val == '0') {
      } else if (val == 'x' || val == 'X') {
        // 000010 // cur_bit
        // 111101 // bitwise complement

        // 110111 // hypothetical current mask
        //&111101 // bitwise and
        //=110101 // new mask, with cur_bit cleared from it
        mask &= ~cur_bit;
      } else {
        throw std::logic_error("Unknown characters in BitPattern input");
      }

      cur_bit >>= 1;
    }
  }

  template<typename ValueType>
  constexpr friend bool operator==(const ValueType value,
                                   const BitPattern &pattern) requires (std::is_unsigned_v<ValueType>){
    return (value & pattern.mask) == pattern.expected;
  }
};
