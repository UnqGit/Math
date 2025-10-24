#include <iostream>
#include <vector>
#include "Math.hpp"
#include "MatrixStatic.hpp"

template <typename T>
void print_matrix(const math::Matrix<T> &mat) noexcept {
    for (const auto &row : mat) {
        for (const auto &elem : row) std::cout << elem << ' ';
        std::cout << '\n';
    }
    std::cout << '\n';
}

template <typename T>
void print_column(const math::Matrix<T> &mat, const size_t col) noexcept {
    try {
        for (const auto &x : mat.column(col)) std::cout << x << '\n';
    } catch(const std::exception &e) {
        std::cerr << "Cannot print because: " << e.what() << '\n';
    }
    std::cout << '\n';
}

template <typename T>
void print_row(const math::Matrix<T> &mat, const size_t row) noexcept {
    try { for (const auto &x : mat.row(row)) std::cout << x << ' '; }
    catch(const std::exception &e) { std::cerr << "Cannot print because: " << e.what() << '\n'; }
    std::cout << '\n';
}

int main(void) {
    math::Matrix2x2<std::string> mat2;
    math::Matrix2x2<int> mat {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) mat += mat;
    std::cout << mat(0, 0) << ' ' << mat(0, 1) << '\n' << mat(1, 0) << ' ' << mat(1, 1);
}