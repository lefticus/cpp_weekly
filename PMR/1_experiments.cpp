#include <fmt/format.h>

#include <memory_resource>
#include <vector>

template <typename ItrBegin, typename ItrEnd>
void print_line(int offset, ItrBegin begin, const ItrEnd end) {
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
void print_buffer(const std::string_view title, const Buffer &buffer,
                  const Container &container) {
  fmt::print("==============={:^10}==============\n", title);

  auto begin = buffer.begin();
  fmt::print("Buffer Address Start: {}\n",
             static_cast<const void *>(buffer.data()));
  for (const auto &elem : container) {
    fmt::print(" Item Address: {}\n", static_cast<const void *>(&elem));
  }
  for (std::size_t offset = 0; offset < buffer.size(); offset += 16) {
    print_line(offset, std::next(begin, offset), std::next(begin, offset + 16));
  }

  fmt::print("\n");
}


void freed_resources() {
  std::array<std::uint8_t, 16> buffer{};
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  print_buffer("initial", buffer, "");
  std::pmr::vector<std::uint8_t> vec1({1, 2, 3, 4}, &mem_resource);
  print_buffer("vec1", buffer, vec1);

  {
    std::pmr::vector<std::uint8_t> vec2({6, 7, 8}, &mem_resource);
    print_buffer("vec1 + vec2", buffer, vec2);
  }  // memory not reclaimed from the buffer

  std::pmr::vector<std::uint8_t> vec3({14, 15, 16}, &mem_resource);
  print_buffer("vec1 + vec3", buffer, vec3);
}

// What happens if the vector data in the monotonic_buffer_resource
// grows and reallocats?
void growing_resources() {
  std::array<std::uint8_t, 16> buffer{};
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  std::pmr::vector<std::uint8_t> vec1(&mem_resource);
  vec1.push_back(1);
  print_buffer("1", buffer, vec1);
  vec1.push_back(2);
  print_buffer("2", buffer, vec1);
  vec1.push_back(3);
  print_buffer("3", buffer, vec1);
  vec1.push_back(4);
  print_buffer("4", buffer, vec1);
  vec1.push_back(5);
  print_buffer("5", buffer, vec1);
}

// short strings, non PMR
void nested_string() {
  fmt::print("Size of `std::pmr::string`: {}\n", sizeof(std::pmr::string));

  std::array<std::uint8_t, 256> buffer{};
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  print_buffer("initial", buffer, "");
  std::pmr::vector<std::pmr::string> vec1(
      {"Hello World", "Hello World 2", "Hello world long string"},
      &mem_resource);
  print_buffer("2 short strings + 1 long string", buffer, vec1);
}

// does not use PMR string
void nested_long_string() {
  std::array<std::uint8_t, 148> buffer{};
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  print_buffer("initial", buffer, "");
  std::pmr::vector<std::string> vec1(&mem_resource);
  vec1.emplace_back("Hello World");
  print_buffer("short string", buffer, vec1);
  vec1.emplace_back("Hello long string here");
  print_buffer("long string", buffer, vec1);
  fmt::print("Notice the moved string\n");
  fmt::print("      vector address: {}\n", static_cast<const void *>(&vec1));
  fmt::print("short string address: {}\n",
             static_cast<const void *>(vec1[0].c_str()));
  fmt::print(" long string address: {}\n",
             static_cast<const void *>(vec1[1].c_str()));
}

// does use PMR string
void nested_long_pmr_string() {
  std::array<std::uint8_t, 160> buffer{};
  std::pmr::monotonic_buffer_resource mem_resource(buffer.data(),
                                                   buffer.size());
  print_buffer("initial", buffer, "");
  std::pmr::vector<std::pmr::string> vec1(&mem_resource);
  vec1.emplace_back("Hello World");
  print_buffer("short string", buffer, vec1);
  vec1.emplace_back("Hello long string here");
  print_buffer("long string", buffer, vec1);
  fmt::print("Notice the moved string\n");
  fmt::print("      vector address: {}\n", static_cast<const void *>(&vec1));
  fmt::print("short string address: {}\n",
             static_cast<const void *>(vec1[0].c_str()));
  fmt::print(" long string address: {}\n",
             static_cast<const void *>(vec1[1].c_str()));
}

int main() {
  nested_string();

  // * know how and if your memory is growing
  // * choose the correct allocator for your use case
  // * make sure you properly nest your allocator aware types
  // * Understand which std types are allocator aware
}
