#include <string>
#include <type_traits>
#include <utility>

template <typename Type>
struct Optional_nontrivial {
  ~Optional_nontrivial() {
    if (static_cast<Type *>(this)->initialized) {
      static_cast<Type *>(this)->data.~Contained();
    }
  }
};

struct Optional_trivial {};

template <typename Contained>
struct Optional
    : std::conditional_t<std::is_trivially_destructible_v<Contained>,
                         Optional_trivial,
                         Optional_nontrivial<Optional<Contained>>> {
  union {
    Contained data;
  };
  bool initialized = false;

  constexpr Optional &operator=(Contained &&data) {
    this->data = std::move(data);
    initialized = true;
    return *this;
  }
};

int main() {
  Optional<int> obj;
  obj = 5;

  static_assert(std::is_trivially_destructible_v<Optional<int>>);
  static_assert(!std::is_trivially_destructible_v<Optional<std::string>>);
}
