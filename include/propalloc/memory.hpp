#ifndef PROPALLOC_MEMORY_H
#define PROPALLOC_MEMORY_H

#include <cstddef>
#include <memory>
#include <properties>

namespace propalloc::memory
{

template<class T, std::size_t Alignment = alignof(std::max_align_t)>
struct layout
{
    using type = T;
    static constexpr auto alignment = Alignment;

    static constexpr bool is_requirable_concept = false;
    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = false;
};

struct reallocation
{
    static constexpr bool is_requirable_concept = false;
    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = true;
};

struct overallocation
{
    static constexpr bool is_requirable_concept = false;
    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = true;
};

template<class Pointer = void*, class SizeType = std::size_t>
struct region
{
    using pointer_type = Pointer;
    using size_type = SizeType;

    Pointer pointer;
    SizeType count;
};

template<class T, std::size_t Align = alignof(T)>
inline constexpr layout<T, Align> storage_for;

template<class T, class... Args>
T*
construct_at(T* p, Args&&... args)
{
    return ::new ((void*)p) T(std::forward<Args>(args)...);
}

} // namespace propalloc::memory

namespace std
{
template<class A, class T, std::size_t Alignment>
struct is_applicable_property<A, ::propalloc::memory::layout<T, Alignment>>
: std::true_type {};

template<class A>
struct is_applicable_property<A, ::propalloc::memory::reallocation>
: std::true_type {};

  template<class A>
  struct is_applicable_property<A, ::propalloc::memory::overallocation>
    : std::true_type {};
}

#endif // PROPALLOC_MEMORY_H
