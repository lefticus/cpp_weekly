#include <fmt/format.h>

#include <memory_resource>
#include <vector>

template <typename ItrBegin, typename ItrEnd>
void print_line(std::int64_t offset, ItrBegin begin, const ItrEnd end) {
  fmt::print("(dec) {:02x}:  {:3}\n", offset, fmt::join(begin, end, "  "));
  fmt::print("(hex) {:02x}:   {:02x}\n", offset, fmt::join(begin, end, "   "));

  fmt::print("(asc) {:02x}:", offset);
  std::for_each(begin, end, [](const auto c) {
    if (std::isgraph(c)) {
      fmt::print("   {} ", static_cast<char>(c));
    } else {
      fmt::print(" \\{:03o}", c);
    }
  });

  fmt::print("\n");
}

template <typename Buffer, typename Container>
void print_buffer(const std::string_view title, const Buffer &buffer, const Container &container) {
  fmt::print("==============={:^10}==============\n", title);

  auto begin = buffer.begin();
  fmt::print("Buffer Address Start: {}\n", static_cast<const void*>(buffer.data()));
  for (const auto &elem : container) {
    fmt::print(" Item Address: {}\n", static_cast<const void *>(&elem));
  }
  for (std::int64_t offset = 0; offset < static_cast<std::int64_t>(buffer.size()); offset += 16) {
    print_line(offset, std::next(begin, offset), std::next(begin, offset + 16));
  }
  
  fmt::print("\n");
}



struct S {
  std::pmr::string str;

  using allocator_type = std::pmr::polymorphic_allocator<>;

  // default constructor, delegate to aa constructor
  S() : S(allocator_type{}) {}
  explicit S(allocator_type alloc)
    : str("Hello long string", alloc)
  {
  }

  S(const S &other, allocator_type alloc = {})
    : str(other.str, alloc)
  {
  }

  S(S &&) = default;

  S(S &&other, allocator_type alloc)
    : str(std::move(other.str), alloc)
  {}

  S &operator=(const S &rhs) = default;
  S &operator=(S &&rhs) = default;

  ~S() = default;

  allocator_type get_allocator() const {
      return str.get_allocator();
  }
};


void aa_type() {
  std::array<std::uint8_t, 304> buffer{};

  fmt::print("Size of object: {}\n", sizeof(S));
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  fmt::print("mem_resource address: {}\n", static_cast<const void *>(&mem_resource));
  print_buffer("initial", buffer, "");
  std::pmr::vector<S> vec1(&mem_resource);
  vec1.push_back(S());
  print_buffer("1 S Object", buffer, vec1);
  vec1.emplace_back();
  print_buffer("2 S Objects", buffer, vec1);
  vec1[0].str = "A different long string";
  print_buffer("Updated string", buffer, vec1);

}

int main() {
  aa_type();  

  // * know how and if your memory is growing
  // * choose the correct allocator for your use case
  // * make sure you properly nest your allocator aware types
  // * Understand which std types are allocator aware 
  // * when building custom types that support allocators
  //     * Make sure to initialize all sub objects correctly (know your defaults)
  //     * Notify the AA containers that we support allocators
}

