#ifndef PROPALLOC_VECTOR_HPP

#include <propalloc/memory.hpp>

#include <type_traits>
#include <stdexcept>

#include <properties>

namespace propalloc
{

namespace detail
{
[[noreturn]] inline
void
throw_allocation_failure()
{
    throw std::bad_alloc();
}

[[noreturn]] inline
void
throw_length_error(char const* err)
{
    throw std::length_error(err);
}

} // namespace detail

template<class T, class Allocator>
    requires std::is_move_constructible_v<T>
class vector
{
public:
    using pointer = T*;
    using reference = T&;

    template <class U>
        requires std::is_constructible_v<T, U const&>
    vector(std::initializer_list<U> list)
    {
        if (list.size() > max_size())
        {
            detail::throw_length_error("vector::vector");
        }
        vector tmp;
        tmp.reserve(list.size());
        std::uninitialized_copy(list.begin(), list.end(), tmp.first_ + tmp.size_);
        tmp.size_ = list.size();
        swap(*this, tmp);
    }

    vector() = default;

    explicit vector(Allocator const& alloc)
      : alloc_{alloc}
    {
    }

    vector(vector const& other)
        requires std::is_copy_constructible_v<T>
      : alloc_{other.alloc_}
    {
        auto alloc = make_alloc();
        auto [p, m] = alloc.allocate(other.size_);
        auto p_t = static_cast<T*>(p);
        std::uninitialized_copy(other.data(), other.size(), p_t);
        first_ = p_t;
        size_ = other.size();
        capacity_ = size_;
    }

    vector(vector&& other) noexcept
      : first_{std::exchange(other.first_, nullptr)}
      , size_{std::exchange(other.size_, 0)}
      , capacity_{std::exchange(other.capacity_, 0)}
      , alloc_{other.alloc_}
    {
    }

    vector&
    operator=(vector&& other) noexcept
    {
        vector tmp{std::move(other)};
        swap(tmp, *this);
        return *this;
    }

    vector&
    operator=(vector const& other)
      requires std::is_copy_constructible_v<T>
    {
        vector tmp{other};
        swap(tmp, *this);
        return *this;
    }

    ~vector()
    {
        deallocate();
    }

    T const*
    data() const noexcept
    {
        return static_cast<T const*>(first_);
    }

    T*
    data() noexcept
    {
        return static_cast<T*>(first_);
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    std::size_t
    max_size() const noexcept
    {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    template<class... Args>
        requires std::is_constructible_v<T, Args...>
    reference
    emplace_back(Args&&... args)
    {
        grow();
        auto* const p = static_cast<T*>(first_ + size_);
        auto& ret = *memory::construct_at(p, std::forward<Args>(args)...);
        size_++;
        return ret;
    }

    template<class Arg>
        requires(std::is_constructible_v<T, Arg> &&
                 std::is_same_v<std::decay_t<Arg>, T>)
    reference
    push_back(Arg&& arg)
    {
        return emplace_back(std::forward<Arg>(arg));
    }

    void
    resize(std::size_t n, T value)
        requires std::is_copy_constructible_v<T>
    {
        if (n > size_)
        {
            reserve(n);
            std::uninitialized_copy_n(first_ + size_, n - size_, std::as_const(value));
            size_ = n;
        }
        else
        {
            std::destroy_n(first_ + n, size_ - n);
            size_ = n;
        }
    }

    void
    resize(std::size_t n)
        requires std::is_default_constructible_v<T>
    {
        resize(n, T());
    }

    void
    reserve(std::size_t n)
    {
        if (capacity_ >= n)
        {
            return;
        }
        if (n > max_size())
        {
            detail::throw_length_error("vector::reserve");
        }
        grow_exact(n);
    }

    template<class U, class A>
    void
    swap(vector<U, A>& l, vector<U, A>& r) noexcept
    {
        using std::swap;
        swap(l.first_, r.first_);
        swap(l.size_, r.size_);
        swap(l.capacity_, r.capacity_);
        swap(l.alloc_, r.alloc_);
    }

private:
    auto
    make_alloc() const noexcept
    {
        return std::prefer(std::require(alloc_, memory::storage_for<T>), memory::reallocation{});
    }

    void
    grow()
    {
        if (capacity_ > size_)
        {
            return;
        }

        if (capacity_ >= max_size() / 2 * 3 - 1)
        {
            detail::throw_length_error("vector::grow");
        }

        grow_exact(2 + capacity_ + capacity_ / 2);
    }

    void
    grow_exact(std::size_t n)
    {
        auto alloc = make_alloc();
        if constexpr (std::is_trivially_copyable_v<T> && std::query(alloc, memory::reallocation{}))
        {
            auto [p, m] = alloc.reallocate({first_, capacity_}, n);
            first_ = static_cast<T*>(p);
            capacity_ = m;
        }
        else
        {
            auto [p, m] = alloc.allocate({first_, n}, n);
            auto* p_t = static_cast<T*>(p);
            try
            {
                if constexpr (std::is_nothrow_move_constructible_v<T>)
                {
                    std::uninitialized_move(first_, size_, p_t);
                }
                else
                {
                    std::uninitialized_copy(first_, size_, p_t);
                }
            }
            catch (...)
            {
                alloc.deallocate({p, m});
                throw;
            }
            using std::swap;
            swap(p, first_);
            swap(m, capacity_);
            std::destroy_n(p_t, size_);
            alloc.deallocate({p, m});
        }
    }

    void
    deallocate() noexcept
    {
        auto alloc = make_alloc();
        std::destroy_n(first_, size_);
        size_ = 0;
        alloc.deallocate({first_, capacity_});
        capacity_ = 0;
        first_ = nullptr;
    }

    pointer first_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
    [[no_unique_address]] Allocator alloc_;
};

} // namespace propalloc

#endif // PROPALLOC_VECTOR_HPP
