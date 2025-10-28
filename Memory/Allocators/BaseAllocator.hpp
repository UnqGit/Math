// BaseAllocator.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

_MMEM_START_
_MTEMPL_ class base_allocator {
    public:
        _NODISC_ T *allocate(const size_t num_elements) const {
            if (num_elements > (static_cast<size_t>(~0) / sizeof(T))) throw _STD_ bad_alloc{};
            return static_cast<T*>(::operator new [sizeof(T) * num_elements]);
        }

        void deallocate(T *&memory, const size_t created_items) const noexcept {
            if constexpr (!_TRV_DSTR_) _STD_ destroy_n(memory, created_items);
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
    using propagate_on_copy_assignment = _STD_ true_type{};
    using propagate_on_move_assignment = _STD_ true_type{};
    using propagate_on_copy_construct  = _STD_ true_type{};
    using propagate_on_move_construct  = _STD_ true_type{};
};
_MATH_END_