#include <algorithm>
#include <array>
#include <cstdio>
#include <type_traits>
#include <vector>


template<typename LHS, typename RHS>
constexpr auto floor_modulo(LHS dividend, RHS divisor) {
  return ((dividend % divisor) + divisor) % divisor;
}

struct Automata {
  using index_t = std::make_signed_t<std::size_t>;

  std::size_t width;
  std::size_t height;
  std::array<bool, 9> born;
  std::array<bool, 9> survives;

  std::vector<bool> data = std::vector<bool>(width * height);

  Automata(std::size_t width_, std::size_t height_, std::array<bool, 9> born_,
           std::array<bool, 9> survives_)
      : width(width_), height(height_), born(born_), survives(survives_) {}

  Automata(std::size_t width_, std::size_t height_, const std::vector<bool> &born_,
           const std::vector<bool> &survives_)
      : width(width_), height(height_) {
         std::copy_n(std::begin(born_), std::min(born_.size(), born.size()), std::begin(born));
         std::copy_n(std::begin(survives_), std::min(survives_.size(), survives.size()), std::begin(survives));
      }


  struct Point {
    index_t x;
    index_t y;

    [[nodiscard]] constexpr Point operator+(Point rhs) const {
      return Point{x + rhs.x, y + rhs.y};
    }
  };

  [[nodiscard]] std::size_t index(Point p) const {
    return floor_modulo(p.y, static_cast<index_t>(height)) * width +
           floor_modulo(p.x, static_cast<index_t>(width));
  }

  [[nodiscard]] bool get(Point p) const { return data[index(p)]; }

  void set(Point p) { data[index(p)] = true; }

  constexpr static std::array<Point, 8> neighbors{
      Point{-1, -1}, Point{0, -1}, Point{1, -1}, Point{-1, 0},
      Point{1, 0},   Point{-1, 1}, Point{0, 1},  Point{1, 1}};

  std::size_t count_neighbors(Point p) const {
    return static_cast<std::size_t>(
        std::count_if(neighbors.begin(), neighbors.end(),
                      [&](auto offset) { return get(p + offset); }));
  }

  [[nodiscard]] Automata next() const {
    Automata result{width, height, born, survives};

    for (std::size_t y = 0; y < height; ++y) {
      for (std::size_t x = 0; x < width; ++x) {
        Point p{static_cast<index_t>(x), static_cast<index_t>(y)};
        const auto neighbors = count_neighbors(p);
        if (get(p)) {
          if (survives[neighbors]) {
            result.set(p);
          }
        } else {
          if (born[neighbors]) {
            result.set(p);
          }
        }
      }
    }

    return result;
  }
  void add_glider(Point p) {
    set(p);
    set(p + Point{1, 1});
    set(p + Point{2, 1});
    set(p + Point{0, 2});
    set(p + Point{1, 2});
  }
};
