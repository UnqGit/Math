#include <iostream>
#include <vector>
#include "Math.hpp"

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
    static_assert(math::isAdditive<int>);
    static_assert(math::isRefAdditive<int>);
    static_assert(math::compoundAddition<int>);
    using namespace math;
    using namespace math::matrix;
    std::cout << "Hello World!\n\n";
    std::cout << std::type_index(typeid(int)).hash_code() << '\n';
    std::cout << std::type_index(typeid(int)).name() << '\n';
    std::cout << std::type_index(typeid(long)).hash_code() << '\n';
    std::cout << std::type_index(typeid(long)).name() << '\n';
    std::cout << "\nBye World!";
}