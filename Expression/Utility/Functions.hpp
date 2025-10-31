// Functions.hpp
#pragma once

namespace math::expr {

enum class Function : char {
    add , sub , mul , div , neg ,       // Basic operators.
    exp , log , logk, pow ,             // Power related functions. // logk is log(x) with arbitrary base.
    sin , cos , tan , csc , sec , cot , // Trig functions.
    asin, acos, atan, acsc, asec, acot, // Inverse Trig functions.
    sinh, cosh, tanh, csch, sech, coth, // Hyperbolic Trig functions.
};
using FN = Function;

inline namespace func {

inline constexpr bool is_binary(const Function val) noexcept {
    switch(val) {
        case FN::add:
        case FN::sub:
        case FN::mul:
        case FN::div:
        case FN::logk:
        case FN::pow:
            return true;
        default:
            return false;
    }
}

inline constexpr bool is_unary(const Function val) noexcept {
    switch(val) {
        case FN::add:
        case FN::sub:
        case FN::mul:
        case FN::div:
        case FN::logk:
        case FN::pow:
            return false;
        default:
            return true;
    }
}

inline constexpr bool is_trig(const Function val) noexcept {
    switch(val) {
        case FN::sin:
        case FN::cos:
        case FN::sec:
        case FN::csc:
        case FN::tan:
        case FN::cot:
            return true;
        default:
            return false;
    }
}

inline constexpr bool is_inv_trig(const Function val) noexcept {
    switch(val) {
        case FN::asin:
        case FN::acos:
        case FN::asec:
        case FN::acsc:
        case FN::atan:
        case FN::acot:
            return true;
        default:
            return false;
    }
}

inline constexpr bool is_hyperbolic(const Function val) noexcept {
    switch(val) {
        case FN::sinh:
        case FN::cosh:
        case FN::sech:
        case FN::csch:
        case FN::tanh:
        case FN::coth:
            return true;
        default:
            return false;
    }
}

}

}