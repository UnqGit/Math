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

#include <unordered_map>

#include <any>

#include <cstddef>
#include <typeindex>
#include <cmath>

#include <memory>

#include <new>
#include <stdexcept>
#include <exception>

#include <omp.h>

#define _STD_               ::std::
#define _RANGES_            ::std::ranges::
#define _MATH_              ::math::
#define _MEM_               ::math::memory::
#define _MEM_IMPL_          ::math::memory::impl::
#define _MATRIX_            ::math::matrix::
#define _MAT_IMPL_          ::math::matrix::impl::
#define _MHELP_             ::math::helper::

#define _MATH_START_        namespace math {
#define _MHELP_START_       namespace math::helper {
#define _MMEM_START_        namespace math::memory {
#define _MMEM_IMPL_START_   namespace math::memory::impl {
#define _MMATRIX_START_     namespace math::matrix {
#define _MMAT_IMPL_START_   namespace math::matrix::impl {
#define _MATH_END_          }

#define _MTEMPL_            template <typename T>
#define _MTYPE_TEMPL(x, y)  template <typename x, typename y>

#define _NODISC_            [[nodiscard]]
#define _DFLT_CSTR_         _STD_ is_default_constructible_v<T>
#define _CPY_CSTR_          _STD_ is_copy_constructible_v<T>
#define _MV_CSTR_           _STD_ is_move_constructible_v<T>
#define _TRV_DSTR_          _STD_ is_trivially_destructible_v<T>
#define _NOTHR_DSTR_        _STD_ is_nothrow_destructible_v<T>
#define _DECL_              _STD_ declval<T>()

#define _ZERO_EXISTS_       const bool zero_exists = _MATH_ zero_vals.exists_of<T>();
#define _GET_ZERO_          _MATH_ zero_vals.get_of<T>()
#define _NO_ZERO_COND_      if constexpr (!_DFLT_CSTR_) if (!zero_exists)

_MATH_START_
_MTEMPL_ using def_ptr      = T *;
_MTEMPL_ using const_ptr    = const T *;
_MTEMPL_ using mut_ptr      = T *const;
_MTEMPL_ using read_ptr     = const T *const;
_MTEMPL_ using read_ptr2d   = const T *const *const;
_MATH_END_