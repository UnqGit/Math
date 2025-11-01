#include "Expression\Utility\Variable\VariableString.hpp"
#include <iostream>

int main(void) {
    using namespace math::literals;
    auto x = "x"_var;
    math::expr::var::VariableName y = "y";
    math::expr::var::VariableName c = 'c';
    std::cout << x.data() << ' ' << y.c_str() << ' ' << c.data();
}