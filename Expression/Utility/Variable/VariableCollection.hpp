// VariableCollection.hpp
#pragma once

#include "VariableValue.hpp"

namespace math::expr {

template <VariableAllowance VarComplexity, Precision ValPrecision>
struct VariableTable;

template <Precision ValPrecision>
struct VariableTable<VA::multi_variable, ValPrecision> {
    using table_type = std::unordered_map
        <
            VariableString,
            VariableValue<ValPrecision>,
            std::hash<std::string_view>
        >;
    table_type m_variable_table;
};

template <Precision ValPrecision>
struct VariableTable<VA::single_variable, ValPrecision> : public VariableValue<ValPrecision> {};

template <Precision ValPrecision>
struct VariableTable<VA::no_variable, ValPrecision> {};

template <VariableAllowance VarComplexity>
struct VariableSet;

struct VariableSet<VA::multi_variable> {
    using set_type = std::unordered_set
        <
            VariableString,
            std::hash<std::string_view>
        >;
    set_type m_variable_set;
};

struct VariableSet<VA::single_variable> {
    VariableString m_var_name;
};

struct VariableSet<VA::no_variable> {};

}