// VariableNameCheck.hpp
#pragma once

#include "VariableUtility.hpp"

namespace math::expr::var {

constexpr inline bool is_upper_case(const char c) noexcept {
    return (c >= 'A' && c <= 'Z');
}

constexpr inline bool is_lower_case(const char c) noexcept {
    return (c >= 'a' && c <= 'z');
}

constexpr inline bool is_digit(const char c) noexcept {
    return (c >= '0' && c <= '9');
}

constexpr inline bool is_alphabet(const char c) noexcept {
    return is_lower_case(c) || is_upper_case(c);
}

constexpr inline bool is_space(const char c) noexcept {
    switch(c) {
        case ' ':
        case '\t':
        case '\n':
        case '\b':
        case '\v':
        case '\r':
        case '\f':
            return true;
        default: return false;
    }
}

enum class VarNameErrType : char {
    no_err, too_small, too_large, first_character_be_letter, has_invalid_char, contains_space, last_letter_underscore, underscore_must_separate_digits, contains_space, is_math_constant, is_math_function, one_char_var_name_be_letter
};
using VNET = VarNameErrType;

constexpr inline VarNameErrType is_valid_var_name(const char c) noexcept { // For variable names of size 1.
    if (!is_alphabet(c)) return VNET::one_char_var_name_be_letter;
    if (c == 'e') return VNET::is_math_constant;
    return VNET::no_err;
}

constexpr VarNameErrType is_valid_var_name(read_ptr<char> data, size_t size) noexcept {
    if (size < 1) return VNET::too_small;
    if (size == 1) return is_valid_var_name(data[0]);
    if (size > 255) return VNET::too_large;
    if (!is_alphabet(data[0])) return VNET::first_character_be_letter;
    for (size_t i = 1; i < size; i++) {
        if (is_alphabet(data[i]));
        else if (data[i] == '_') {
            if (i + 1 == size) return VNET::last_letter_underscore;
        }
        else if (is_digit(data[i])) {
            if (data[i - 1] != '_') return VNET::underscore_must_separate_digits;
            while (++i < size && is_digit(data[i]));
            if (i < size) {
                if (data[i] != '_') return VNET::underscore_must_separate_digits;
                else if (i == size - 1) return VNET::last_letter_underscore;
            }
        }
        else if (is_space(data[i])) return VNET::contains_space;
        else return VNET::has_invalid_char;
    }
    switch (size) {
        case 2:
            if (data[0] == 'p' && data[1] == 'i') return VNET::is_math_constant;
            break;
        case 3:
        case 4:
            if (is_math_func(std::string_view(data, size))) return VNET::is_math_function;
            break;
        default: break;
    }
    return VNET::no_err;
}

}