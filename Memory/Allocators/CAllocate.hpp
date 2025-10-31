// CAllocate.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

namespace math::memory::impl {
using aligned_alloc_t = void* (*)(size_t, size_t);
using free_t = void(*)(void*);
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
    #include <malloc.h>
    inline void *msc_aligned_alloc(size_t alignment, size_t size) {
        return _aligned_malloc(size, alignment);
    }
    aligned_alloc_t aligned_allocate = msc_aligned_alloc;
    free_t free = _aligned_free;
#else
    aligned_alloc_t aligned_allocate = std::aligned_alloc;
    free_t free = std::free;
#endif
}

namespace math::memory {
_MTEMPL_ class basic_allocator {
    public:
        _NODISC_ T *allocate(const size_t num_elements) const {
            static constexpr const size_t align(alignof(T));
            static constexpr const size_t size(sizeof(T));
            if (num_elements > (static_cast<size_t>(~0) / size)) throw std::bad_alloc{};
            size_t bytes = size * num_elements;
            T *ptr;
            if (bytes == 0) ptr = static_cast<T*>(std::malloc(1));
            else if constexpr (align > alignof(std::max_align_t)) ptr = static_cast<T*>(math::memory::impl::aligned_alloc(align, bytes));
            else ptr = static_cast<T*>(std::malloc(bytes));        
            if (ptr) [[likely]] return ptr;
            else throw std::bad_alloc{};
        }

        void deallocate(T *&memory, const size_t created_items) const noexcept {
            if constexpr (!TrvDtor<T>) std::destroy_n(memory, created_items);
            if (memory) {
                if constexpr (alignof(T) > alignof(std::max_align_t)) math::memory::impl::free(memory);
                else std::free(memory);
                memory = nullptr;
            }
        }

    public:
        _MTMPLU_ _NODISC_ constexpr bool operator==(const basic_allocator<U> &other) const noexcept {
            return true;
        }

        _MTMPLU_ _NODISC_ constexpr bool operator!=(const basic_allocator<U> &other) const noexcept {
            return false;
        }
};

_MTEMPL_ struct allocator_traits<T, basic_allocator<T>> {
    using value_type = T;
    using propagate_on_copy_assignment = std::true_type{};
    using propagate_on_move_assignment = std::true_type{};
    using propagate_on_copy_construct  = std::true_type{};
    using propagate_on_move_construct  = std::true_type{};
};
}