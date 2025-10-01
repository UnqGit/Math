#include "Math.hpp"
#include <iostream>

using namespace math::helper;

int main(void) {
    zero_vals.store_of(0);
    zero_vals.store_of(10.0f);
    math::matrix::Order order(30, 30);
    std::cout << zero_vals.get_of<int>() << ' ' << zero_vals.get_of<float>() << std::endl;
    std::cout << (order == math::matrix::Order(90, 20)) << std::endl;
    std::cout << sizeof(decltype(order)) << std::endl;
    std::cout << sizeof(math::Matrix<int>) << std::endl;
    std::cout << alignof(math::Matrix<int>) << std::endl;
}