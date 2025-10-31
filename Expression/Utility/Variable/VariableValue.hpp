// VariableValue.hpp
#pragma once

#include "VariableString.hpp"

namespace math::expr {

enum class Precision : char {
    less, regular, high
};
using PC = Precision;

enum class VariableAllowance : char {
    no_variable, single_variable, multi_variable
};
using VA = VariableAllowance;

template <Precision ValPrecision>
struct VariableValue;

struct VariableValue<PC::less> {
    float val;
    constexpr VariableValue() noexcept : val(0.0f) {}
    constexpr VariableValue(float base) noexcept : val(base) {}
    constexpr operator float() noexcept {
        return val;
    }
};

struct VariableValue<PC::regular> {
    double val;
    constexpr VariableValue() noexcept : val(0.0) {}
    constexpr VariableValue(double base) noexcept : val(base) {}
    constexpr operator double() noexcept {
        return val;
    }
};

struct VariableValue<PC::high> {
    long double val;
    constexpr VariableValue() noexcept : val(0.0L) {}
    constexpr VariableValue(const long double &base) noexcept : val(base) {}
    constexpr operator long double() noexcept {
        return val;
    }
};

template <Precision ValPrecision>
struct Variable {
    VariableString name;
    VariableValue<ValPrecision> val;
};

}