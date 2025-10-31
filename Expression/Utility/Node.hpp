// Node.hpp
#pragma once

#include "Variable\VariableValue.hpp"

namespace math::expr {

enum class NodeType : char {
    num, func, var
};
using NT = NodeType;

template <VariableAllowance VarComplexity, Precision ValPrecision>
struct NodeValue {
    using value_type = std::conditional_t
        <
            VarComplexity == VA::multi_variable,
            std::variant<VariableValue<ValPrecision>, Function, VariableString>,
            std::variant<VariableValue<ValPrecision>, Function>,
        >;
    value_type val;
    NodeType  type;
};

template <VariableAllowance VarComplexity, Precision ValPrecision>
struct MathNode : NodeValue<VarComplexity, ValPrecision> {
    using next_node = mut_ptr<MathNode<VarComplexity, ValPrecision> >;
    next_node  lhs ;
    next_node  rhs ;
};

}