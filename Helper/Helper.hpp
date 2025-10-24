// Helper.hpp
#pragma once

#include "Headers.hpp"

_MATH_START_
_MTEMPL_ concept ProperEquality = requires(const T &a, const T &b) {
    { a == b } -> _STD_ same_as<bool>;
};

// Equality checking functions.
template <_STD_ floating_point T>
inline constexpr bool is_equal(const T &a, const T &b) noexcept {
    static constexpr const T rel_tol(static_cast<T>(4) * _STD_ numeric_limits<T>::epsilon());
    static constexpr const T abs_tol(_STD_ numeric_limits<T>::denorm_min());
    return (_STD_ fabs(a - b) <= _STD_ max(rel_tol * _STD_ max(_STD_ fabs(a), _STD_ fabs(b)), abs_tol));
}

_MTEMPL_ requires (!_STD_ floating_point<T> && _MATH_ ProperEquality<T>)
inline constexpr bool is_equal(const T &a, const T &b) noexcept( _DECL_ == _DECL_ ) {
    return a == b;
}

_MTEMPL_ concept isEqualityOperationPossible = requires(const T &a, const T &b) {
    _MATH_ is_equal(a, b);
};

_MTEMPL_ concept isAdditive = requires(const T &a, const T &b) {
    requires _STD_ same_as<_STD_ remove_const_t<decltype(a + b)>, T>;
};

_MTEMPL_ concept isRefAdditive = requires(T &a, const T &b) {
    { a += b } -> _STD_ same_as<T&>;
};

_MTEMPL_ concept compoundAddition = _MATH_ isAdditive<T> && _MATH_ isRefAdditive<T>;

_MTEMPL_ concept anyAddition = _MATH_ isAdditive<T> || _MATH_ isRefAdditive<T>;

_MTEMPL_ concept isSubtractible = requires(const T &a, const T &b) {
    requires _STD_ same_as<_STD_ remove_const_t<decltype(a - b)>, T>;
};

_MTEMPL_ concept isRefSubtractible = requires(T &a, const T &b) {
    { a -= b } -> _STD_ same_as<T&>;
};

_MTEMPL_ concept compoundSubtraction = _MATH_ isSubtractible<T> && _MATH_ isRefSubtractible<T>;

_MTEMPL_ concept anySubtraction = _MATH_ isSubtractible<T> || _MATH_ isRefSubtractible<T>;

_MTEMPL_ concept isMultiplicative = requires(const T &a, const T &b) {
    requires _STD_ same_as<_STD_ remove_const_t<decltype(a * b)>, T>;
};

_MTEMPL_ concept isRefMultiplicative = requires(T &a, const T &b) {
    { a *= b } -> _STD_ same_as<T&>;
};

_MTEMPL_ concept compoundMultiplication = _MATH_ isMultiplicative<T> && _MATH_ isRefMultiplicative<T>;

_MTEMPL_ concept anyMultiplication = _MATH_ isMultiplicative<T> || _MATH_ isRefMultiplicative<T>;

_MTYPE_TEMPL(T, ...Args) concept allSameType = _STD_ conjunction_v<_STD_ is_same<T, Args>...>;

// Zero value holder class.
class ZeroValueHolder {
    public:
        static ZeroValueHolder &instance() noexcept {
            static ZeroValueHolder zero_vals;
            return zero_vals;
        }
    public:
        _MTEMPL_ constexpr void store_of(const T &val) { // Cannot do noexcept because typeid can throw.
            m_vals[_STD_ type_index(typeid(T))] = val;
        }
        _MTEMPL_ _NODISC_ constexpr bool exists_of() {
            return (m_vals.find(_STD_ type_index(typeid(T))) != m_vals.end());   
        }
        _MTEMPL_ _NODISC_ constexpr const T &get_of() const {
            const auto loc = m_vals.find(_STD_ type_index(typeid(T)));
            if (loc != m_vals.end()) return _STD_ any_cast<const T&>(loc->second);
            else throw _STD_ logic_error("Cannot provide the zero value of a type that is not already stored in _MATH_ helper::zero_vals.\n");
        }
    private:
        _STD_ unordered_map<_STD_ type_index, _STD_ any> m_vals;
    private:
        constexpr ZeroValueHolder() { // Helping the file user by pre-saving some of the types' 0 val.
            store_of<char>(static_cast<char>(0));
            store_of<short>(static_cast<short>(0));
            store_of<int>(static_cast<int>(0));
            store_of<long>(static_cast<long>(0));
            store_of<long long>(static_cast<long long>(0));
            store_of<unsigned char>(static_cast<unsigned char>(0));
            store_of<unsigned short>(static_cast<unsigned short>(0));
            store_of<unsigned int>(static_cast<unsigned int>(0));
            store_of<unsigned long>(static_cast<unsigned long>(0));
            store_of<unsigned long long>(static_cast<unsigned long long>(0));
            store_of<const char>(static_cast<const char>(0));
            store_of<const short>(static_cast<const short>(0));
            store_of<const int>(static_cast<const int>(0));
            store_of<const long>(static_cast<const long>(0));
            store_of<const long long>(static_cast<const long long>(0));
            store_of<const unsigned char>(static_cast<const unsigned char>(0));
            store_of<const unsigned short>(static_cast<const unsigned short>(0));
            store_of<const unsigned int>(static_cast<const unsigned int>(0));
            store_of<const unsigned long>(static_cast<const unsigned long>(0));
            store_of<const unsigned long long>(static_cast<const unsigned long long>(0));
            store_of<float>(static_cast<float>(0));
            store_of<double>(static_cast<double>(0));
            store_of<long double>(static_cast<long double>(0));
            store_of<const float>(static_cast<const float>(0));
            store_of<const double>(static_cast<const double>(0));
            store_of<const long double>(static_cast<const long double>(0));
            
            store_of<char*>(nullptr);
            store_of<short*>(nullptr);
            store_of<int*>(nullptr);
            store_of<long*>(nullptr);
            store_of<long long*>(nullptr);
            store_of<unsigned char*>(nullptr);
            store_of<unsigned short*>(nullptr);
            store_of<unsigned int*>(nullptr);
            store_of<unsigned long*>(nullptr);
            store_of<unsigned long long*>(nullptr);
            store_of<const char*>(nullptr);
            store_of<const short*>(nullptr);
            store_of<const int*>(nullptr);
            store_of<const long*>(nullptr);
            store_of<const long long*>(nullptr);
            store_of<const unsigned char*>(nullptr);
            store_of<const unsigned short*>(nullptr);
            store_of<const unsigned int*>(nullptr);
            store_of<const unsigned long*>(nullptr);
            store_of<const unsigned long long*>(nullptr);
            store_of<float*>(nullptr);
            store_of<double*>(nullptr);
            store_of<long double*>(nullptr);
            store_of<const float*>(nullptr);
            store_of<const double*>(nullptr);
            store_of<const long double*>(nullptr);

            store_of<void*>(nullptr);
            store_of<const void*>(nullptr);
        }; // Private default constructor.
        ZeroValueHolder(const ZeroValueHolder&) = delete;
        ZeroValueHolder& operator=(const ZeroValueHolder&) = delete;
};
ZeroValueHolder& zero_vals = _MATH_ ZeroValueHolder::instance();
_MATH_END_

_MHELP_START_
_MTYPE_TEMPL(Container, RequiredData) concept isOneDArr =
    _RANGES_ sized_range<Container> &&
    _STD_ same_as<
        _STD_ decay_t<_RANGES_ range_value_t<Container>>,
        _STD_ decay_t<RequiredData>
    >;

_MTYPE_TEMPL(Container, RequiredData) concept isTwoDArr =
    _RANGES_ sized_range<Container> &&
    isOneDArr<
        _STD_ decay_t<_RANGES_ range_value_t<Container>>,
        RequiredData
    >;
_MATH_END_