// CAllocate.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

_MMEM_IMPL_START_
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
    aligned_alloc_t aligned_allocate = _STD_ aligned_alloc;
    free_t free = _STD_ free;
#endif
_MATH_END_

_MMEM_START_
_MTEMPL_ class basic_allocator {
    public:
        _NODISC_ T *allocate(const size_t num_elements) const {
            static constexpr const size_t align(alignof(T));
            static constexpr const size_t size(sizeof(T));
            if (num_elements > (static_cast<size_t>(~0) / size)) throw _STD_ bad_alloc{};
            size_t bytes = size * num_elements;
            T *ptr;
            if (bytes == 0) ptr = static_cast<T*>(_STD_ malloc(1));
            else if constexpr (align > alignof(_STD_ max_align_t)) ptr = static_cast<T*>(_MEM_IMPL_ aligned_alloc(align, bytes));
            else ptr = static_cast<T*>(_STD_ malloc(bytes));        
            if (ptr) [[likely]] return ptr;
            else throw _STD_ bad_alloc{};
        }

        void deallocate(T *&memory, const size_t created_items) const noexcept {
            if constexpr (!_TRV_DSTR_) _STD_ destroy_n(memory, created_items);
            if (memory) {
                if constexpr (alignof(T) > alignof(_STD_ max_align_t)) _MEM_IMPL_ free(memory);
                else _STD_ free(memory);
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
    using propagate_on_copy_assignment = _STD_ true_type{};
    using propagate_on_move_assignment = _STD_ true_type{};
    using propagate_on_copy_construct  = _STD_ true_type{};
    using propagate_on_move_construct  = _STD_ true_type{};
};
_MATH_END_