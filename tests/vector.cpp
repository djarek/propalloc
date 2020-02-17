
#include <propalloc/vector.hpp>

#include <boost/core/lightweight_test.hpp>

namespace detail
{

inline
propalloc::memory::region<>
internal_reallocate(propalloc::memory::region<> m, std::size_t count, std::size_t size)
{
    auto* p = std::realloc(m.pointer, count * size);
    if (p == nullptr)
        propalloc::detail::throw_allocation_failure();
    return {p, count};
}
} // namespace detail

template<std::size_t Size = 0, bool Relocatable = false, bool ReallocationEnabled = false>
struct mallocator
{
    propalloc::memory::region<>
    allocate(std::size_t count) const
        requires(Size > 0)
    {
        return detail::internal_reallocate({}, count, Size);
    }

    propalloc::memory::region<>
    reallocate(propalloc::memory::region<> m, std::size_t count) const
        requires (Size > 0 && Relocatable && ReallocationEnabled)
    {
        return detail::internal_reallocate(m, count, Size);
    }

    void
    deallocate(propalloc::memory::region<> m) const noexcept
        requires (Size > 0)
    {
        std::free(m.pointer);
    }

    template <class T, std::size_t A>
    auto
    require(propalloc::memory::layout<T, A>) const noexcept
        requires (A <= alignof(std::max_align_t))
    {
        return mallocator<sizeof(T), std::is_trivially_copyable_v<T>, ReallocationEnabled>{};
    }

    auto
    require(propalloc::memory::reallocation) const noexcept
    {
        return mallocator<Size, Relocatable, true>{};
    }

    constexpr
    bool
    query(propalloc::memory::reallocation)
    {
        return Relocatable && ReallocationEnabled;
    }

    bool
    operator==(mallocator const&) const = default;
};

static_assert(std::is_applicable_property_v<mallocator<>, propalloc::memory::layout<int, 4>>);
static_assert(std::is_applicable_property_v<mallocator<>, propalloc::memory::reallocation>);

static_assert(!std::query(mallocator<>{}, propalloc::memory::reallocation{}));

static_assert(!std::query(mallocator<4, true, false>{}, propalloc::memory::reallocation{}));
static_assert(!std::query(mallocator<4, false, true>{}, propalloc::memory::reallocation{}));
static_assert(std::query(mallocator<4, true, true>{}, propalloc::memory::reallocation{}));

namespace
{

void
test_init_list()
{
    propalloc::vector<int, mallocator<>> vec{1, 2, 3, 4};

}

void
test_push_back()
{
    propalloc::vector<int, mallocator<>> vec;

    vec.reserve(523);
}

} // namespace


int
main()
{
    test_init_list();
    test_push_back();
}
