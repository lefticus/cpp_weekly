#pragma once

#include <memory_resource>
#include <string>

namespace gc {
    template<class Tag/* to generate a new type to use a different memory_resource */>
    inline thread_local typename Tag::memory_resource_type *mr = nullptr;

    template<class T, class Tag/* to generate a new type to use a different memory_resource */>
    class allocator {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::true_type;
        using is_always_equal = std::true_type;

        allocator() = default;

        template<class U>
        constexpr explicit allocator(const allocator<U, Tag> &) noexcept {
        }

        [[nodiscard]] T *allocate(size_t n);

        void deallocate(T *p, size_t n);
    };

    template<class T, class U, class Tag>
    [[nodiscard]] constexpr bool
    operator==(allocator<T, Tag> const & /*unused*/, allocator<U, Tag> const & /*unused*/) noexcept {
        return true;
    }

    template<class T, class Tag>
    T *allocator<T, Tag>::allocate(const size_t n) {
        return static_cast<T *>(mr<Tag>->allocate(n * sizeof(T), alignof(T)));
    }

    template<class T, class Tag>
    void allocator<T, Tag>::deallocate(T *p, const size_t n) {
        mr<Tag>->deallocate(p, n * sizeof(T), alignof(T));
    }



    template<
        class Tag,
        class CharT,
        class Traits = std::char_traits<CharT> >
    using basic_string =
    std::basic_string<CharT, Traits, gc::allocator<CharT, Tag> >;

    template<class Tag>
    using string = basic_string<Tag, char>;


    struct monotonic_buffer_resource_devirt final : std::pmr::monotonic_buffer_resource {
        // this class is needed to help the compiler to devirtualize the calls
        using monotonic_buffer_resource::monotonic_buffer_resource;

        void *allocate(size_t bytes, size_t align) {
            return monotonic_buffer_resource::do_allocate(bytes, align);
        }

        void deallocate(void *p, std::size_t bytes, std::size_t alignment) {
            monotonic_buffer_resource::do_deallocate(p, bytes, alignment);
        };
    };
}
