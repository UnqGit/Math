// Headers.hpp
#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include <algorithm>
#include <utility>

#include <functional>
#include <limits>
#include <cstdlib>

#include <unordered_set>
#include <unordered_map>

#include <any>
#include <variant>

#include <string>
#include <string_view>
#include <cstring>

#include <cstddef>
#include <typeindex>
#include <cmath>

#include <memory>

#include <new>
#include <stdexcept>
#include <exception>

#include <omp.h>

#define _MTEMPL_            template <typename T>
#define _MTMPLU_            template <typename U>
#define _MTYPE_TEMPL(x, y)  template <typename x, typename y>

#define _NODISC_            [[nodiscard]]
#define _DECL_              std::declval<T>()

namespace math {
_MTEMPL_ using mut_ptr      = T *;
_MTEMPL_ using const_ptr    = const T *;
_MTEMPL_ using fixed_ptr    = T *const;
_MTEMPL_ using read_ptr     = const T *const;
_MTEMPL_ using read_ptr2d   = const T *const *const;
}

namespace math {
_MTEMPL_ concept DfltCtor  = std::is_default_constructible_v<T>;
_MTEMPL_ concept CpyCtor   = std::is_copy_constructible_v<T>;
_MTEMPL_ concept MvCtor    = std::is_move_constructible_v<T>;
_MTEMPL_ concept TrvDtor   = std::is_trivially_destructible_v<T>;
_MTEMPL_ concept NothrDtor = std::is_nothrow_destructible_v<T>;
}

#define _ZERO_EXISTS_       const bool zero_exists = math::zero_vals.exists_of<T>();
#define _GET_ZERO_          math::zero_vals.get_of<T>()
#define _NO_ZERO_COND_      if constexpr (!DfltCtor<T>) if (!zero_exists)

namespace math::memory {
_MTYPE_TEMPL(T, Allocator) struct allocator_traits;
}