// BtreeImpl.hpp
#pragma once

#include "..\Utility\Node.hpp"

namespace math::expr::btree {

template <VariableAllowance VarComplexity, Precision ValPrecision>
inline void destroy_btree_node(mut_ptr<MathNode<VarComplexity, ValPrecision> > p_node) noexcept {
    if (p_node == nullptr) return;
    destroy_btree_node(p_node->lhs);
    destroy_btree_node(p_node->rhs);
    delete p_node;
}

}
