#include <iostream>

#define DUMP_MACRO(x) std::cout << #x " " << x << '\n'

template<typename T, std::size_t N>
struct MyData {
  T data[N];
};

void go() {
  std::cout << "Hello World\n";

  DUMP_MACRO(__cpp_constexpr);
  DUMP_MACRO(__clang_major__);
  DUMP_MACRO(__clang_minor__);
  DUMP_MACRO(__cpp_init_captures);
}

