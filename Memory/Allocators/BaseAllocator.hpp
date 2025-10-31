// BaseAllocator.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

namespace math::memory {
_MTEMPL_ class base_allocator {
    public:
        _NODISC_ T *allocate(const size_t num_elements) const {
            if (num_elements > (static_cast<size_t>(~0) / sizeof(T))) throw std::bad_alloc{};
            return static_cast<T*>(::operator new [sizeof(T) * num_elements]);
        }

        void deallocate(T *&memory, const size_t created_items) const noexcept {
            if constexpr (!TrvDtor<T>) std::destroy_n(memory, created_items);
            ::operator delete[](memory);
            memory = nullptr;
        }

    public:
        _MTMPLU_ constexpr bool operator==(const base_allocator<U> &other) const noexcept {
            return true;
        }

        _MTMPLU_ constexpr bool operator!=(const base_allocator<U> &other) const noexcept {
            return false;
        }
};

_MTEMPL_ struct allocator_traits<T, base_allocator<T>> {
    using value_type = T;
    using propagate_on_copy_assignment = std::true_type{};
    using propagate_on_move_assignment = std::true_type{};
    using propagate_on_copy_construct  = std::true_type{};
    using propagate_on_move_construct  = std::true_type{};
};
}