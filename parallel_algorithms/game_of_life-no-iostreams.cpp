#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <cstdio>

// This is a simple conway's game-of-life implementation
// that is constexpr friendly and can work as a benchmark
// for parallel computation models in C++
//
// Notes I learned along the way while learning AdaptiveCpp
//
// AMD GPU Install notes:
//  * AMD focuses on LTS ubuntu releases, if you have a different release,
//  expect a little pain
//  * I had good luck installing the AMDGPU Installer option here:
//    https://rocm.docs.amd.com/projects/install-on-linux/en/latest/tutorial/quick-start.html#amdgpu-ubuntu
//  * The rocm-gdb package would not install on my OS because of some outdated
//  dependencies
//  * The amdgpu-install tool will set up the apt repositories that you need
//  * If your OS is fully supported, just install the copy level package
//  * Honestly, I just kept installing random ROCm packages until I got things
//  working,
//    which was I think everything except for the gdb package that I could not
//    install
//
//     After You've Installed ROCm
//      * add yourself to the render group
//      * consider rebooting probably
//      * run `rocminfo` and make sure it sees your GPUs
//
// Other GPUs:
//  * I have no input here
//
// Use the "automatic installation script" to install llvm >= 14
//  * https://apt.llvm.org/
//  * You probably want to install "all"
//   ```sh
//   wget https://apt.llvm.org/llvm.sh
//   chmod +x llvm.sh
//   sudo ./llvm.sh <version number> all
//   ```
//
// Now Build And Install AdaptiveCpp
//  * https://github.com/AdaptiveCpp/AdaptiveCpp/blob/develop/doc/installing.md#a-standard-installation
//  * Run `acpp-info` and make sure you get output similar to what `rocminfo`
//  gave you
//
// Install nvtop to monitor GPU usage and make sure this is doing what you want.
//
// To Compare with GCC
//   * install libttb-dev
//
// Theoretically you are ready to go now?!
//
//
// To compile with all optimizations and parallel std lib support enabled:
//
// ```sh
// # AdaptiveCpp
// acpp -std=c++23 ./game_of_life.cpp -O3 -march=native --acpp-stdpar
//
// # gcc/clang. If you don't have ttb installed/linked it falls back to single
// threaded silently g++ -std=c++23 ./game_of_life.cpp -O3 -march=native -lttb
// clang++ -std=c++23 ./game_of_life.cpp -O3 -march=native -lttb
//
// # Depending on clang version you might need to add -fexperimental-library
// ```
//
// Run, watch nvtop, htop, run with /usr/bin/time to see total CPU utilization,
// etc and see how it scales on your platform

// Handy modulo operator that wraps around automatically
[[nodiscard]] constexpr auto floor_modulo(auto dividend, auto divisor) {
  return ((dividend % divisor) + divisor) % divisor;
}

// This is probably unnecessary, but the min_int
// utilities exist to make the `Point` type as compact as possible
// so that we only use int16 if that's all we need, for example
template <std::size_t value> auto min_int() {
  if constexpr (value <= std::numeric_limits<std::int8_t>::max()) {
    return std::int8_t{};
  } else if constexpr (value <= std::numeric_limits<std::int16_t>::max()) {
    return std::int16_t{};
  } else if constexpr (value <= std::numeric_limits<std::int32_t>::max()) {
    return std::int32_t{};
  } else {
    return std::int64_t{};
  }
}

template <std::size_t value> using min_int_t = decltype(min_int<value>());

// templated on size mostly to give the compiler extra hints
// about the code, so it knows what it can unroll, etc.
template <std::size_t Width, std::size_t Height> struct GameBoard {
  // These are the properly sized things necessary to hold coordinates
  // that work with this particular size of board
  using x_index_t = min_int_t<Width>;
  using y_index_t = min_int_t<Height>;

  static constexpr x_index_t width = Width;
  static constexpr y_index_t height = Height;

  std::array<bool, Width * Height> data;

  struct Point {
    x_index_t x;
    y_index_t y;
    [[nodiscard]] constexpr Point operator+(Point rhs) const {
      return Point{static_cast<x_index_t>(x + rhs.x),
                   static_cast<y_index_t>(y + rhs.y)};
    }
  };

  // The 8 relative positions for neighbors for a given point
  constexpr static std::array<Point, 8> neighbors{
      Point{-1, -1}, Point{0, -1}, Point{1, -1}, Point{-1, 0},
      Point{1, 0},   Point{-1, 1}, Point{0, 1},  Point{1, 1}};

  // Takes the input point, wraps it vertically/horizontally and takes
  // the new location and maps that to the linear address of the point
  // in the underlying array
  [[nodiscard]] constexpr static std::size_t index(Point p) {
    return static_cast<std::size_t>(floor_modulo(p.y, height) * width +
                                    floor_modulo(p.x, width));
  }

  [[nodiscard]] constexpr bool operator[](Point p) const noexcept {
    return data[index(p)];
  }

  constexpr void set(Point p) noexcept { data[index(p)] = true; }

  [[nodiscard]] constexpr std::size_t count_neighbors(Point p) const {
    return static_cast<std::size_t>(
        std::count_if(neighbors.begin(), neighbors.end(),
                      [&](auto offset) { return (*this)[p + offset]; }));
  }

  // Pre-compute all of the Point coordinates that exist in this particular
  // gameboard. We use this later to iterate over every location in the
  // gameboard.
  [[nodiscard]] static auto make_indexes() {
    auto result = std::make_unique<std::array<Point, Width * Height>>();

    std::size_t output_index = 0;

    for (y_index_t y = 0; y < height; ++y) {
      for (x_index_t x = 0; x < width; ++x) {
        (*result)[output_index] = Point{x, y};
        ++output_index;
      }
    }
    return result;
  };

  // https://en.wikipedia.org/wiki/Conway's_Game_of_Life#Examples_of_patterns

  // Add a glider at a given location on the game board
  constexpr void add_glider(Point p) {
    set(p);
    set(p + Point{1, 1});
    set(p + Point{2, 1});
    set(p + Point{0, 2});
    set(p + Point{1, 2});
  }
};

template <typename BoardType>
constexpr void iterate_board(const BoardType &input, BoardType &output,
                             auto &indices) {

  const auto rules = [&](const auto &index) {
    const auto neighbor_count = input.count_neighbors(index);
    const auto is_alive = input[index];

    if (is_alive) {
      if (neighbor_count < 2) {
        return false;
      } else if (neighbor_count <= 3) {
        return true;
      } else {
        return false;
      }
    } else {
      if (neighbor_count == 3) {
        return true;
      } else {
        return false;
      }
    }

    return true;
  };

  std::transform(indices.begin(), indices.end(), output.data.begin(), rules);
}


template <typename BoardType> auto print_board(const BoardType &board) {
  for (int y = 0; y < board.height; ++y) {
    for (int x = 0; x < board.width; ++x) {
      if (board[typename BoardType::Point(x, y)]) {
        putchar('*');
      } else {
        putchar(' ');
      }
    }
    putchar('\n');
  }
}

template <std::size_t Width, std::size_t Height, std::size_t Iterations>
void run_board() {
  using board_type = GameBoard<Width, Height>;

  // I would consider putting these on the stack, but the GPU engine
  // requires pointers that it knows how to work with. With AdaptiveCpp
  // it swaps out malloc and owns these pointers in a way that can be used
  // with the GPU automagically

  auto board1 = std::make_unique<board_type>();
  board1->add_glider(typename board_type::Point(1, 3));
  board1->add_glider(typename board_type::Point(10, 1));
  auto board2 = std::make_unique<board_type>();

  const auto indices = board_type::make_indexes();

  {
    for (int i = 0; i < Iterations; ++i) {
      // just swapping buffers back and forth
      iterate_board(*board1, *board2, *indices);
      std::swap(board1, board2);
    }
  }

    // this exists solely to make sure the compiler doesn't optimize out the
    // actual work
    if ((*board1)[typename board_type::Point(0, 0)]) {
      puts("0,0 is Set!");
    } else {
      puts("0,0 is Not Set!");
    }
}

int main() {
  run_board<10, 10, 5'000'000>();
  run_board<100, 10, 500'000>();
  run_board<100, 100, 50'000>();
  run_board<100, 1000, 5'000>();
  run_board<1000, 1000, 500>();
  run_board<10000, 1000, 50>();
  run_board<10000, 10000, 5>();
}
