// VariableUtility.hpp
#pragma once

#include "..\..\..\..\Helper\Headers.hpp"

namespace math::expr::var {

const std::unordered_set<std::string_view> unicode_operators {
    "÷", "√", "·", "×",
};

const std::unordered_set<std::string_view> function_names {
    "exp" , "log" , "logk", "pow" , "sqrt", "cbrt",
    "sin" , "cos" , "tan" , "csc" , "sec" , "cot" ,
    "asin", "acos", "atan", "acsc", "asec", "acot",
    "sinh", "cosh", "tanh", "csch", "sech", "coth",
};

bool is_math_func(std::string_view s) noexcept {
    switch(s.size()) {
        case 3:
        case 4:
           return (function_names.find(s) != function_names.end());
        default: return false; 
    }
}

constexpr bool is_math_operator(const char c) noexcept {
    switch(c) {
        case '+':
        case '-':
        case '*':
        case '/':
            return true;
        default: return false;
    }
}

bool is_unicode_math_operator(std::string_view s) noexcept {
    switch(s.size()) {
        case 2:
        case 3:
            return (unicode_operators.find(s) != unicode_operators.end());
        default: return false;
    }
}

bool is_math_operator(std::string_view s) noexcept {
    switch(s.size()) {
        case 1: return is_math_operator(s[0]);
        case 2:
        case 3:
            return is_unicode_math_operator(s);
        default: return false;
    }
}

}