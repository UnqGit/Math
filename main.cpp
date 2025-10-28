#include <iostream>
#include <vector>
#include "Math.hpp"
#include <array>

int main(void) {
    math::Matrix2x2<std::string> mat2;
    math::Matrix2x2<int> mat {1, 2, 3, 4};
    math::Matrix<int> mat3(90);
    for (int i = 0; i < 4; i++) mat += mat;
    std::cout << mat(0, 0) << ' ' << mat(0, 1) << '\n' << mat(1, 0) << ' ' << mat(1, 1);
    std::cout << "\n\n" << sizeof(math::Matrix2x2<int>) << '\n';
    std::cout << sizeof(std::unique_ptr<std::array<int, 10>>);
}