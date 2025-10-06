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

int main(void) {
    using namespace math;
    using namespace math::matrix;
    std::cout << "Hello World!\n\n";
    // print_matrix<int> (Matrix<int>(5, 6, 7, math::matrix::CSR::alternate));
    // print_matrix<int> (Matrix<int>(5, 6, 7, math::matrix::CSR::alternate_row));
    // print_matrix<int> (Matrix<int>(5, 6, 7, math::matrix::CSR::alternate_column));
    // print_matrix<int> (Matrix<int>(9, 7, 8));
    // std::cout << Matrix<int>(9, 7, 8).size() << "\n\n";
    // print_matrix<int> (Matrix<int>(Order(6, 6), [](){
    //     static int i = 0;
    //     return i++;
    // }).transpose_in_place());
    Matrix<int> mat(5, 6, CSR::alternate);
    print_column<int> (mat, 2);
    for (auto &x : mat.column(2)) x += 2;
    print_matrix<int> (mat);
    for (auto x : mat.row(2)) x += 2;
    print_matrix<int> (mat);
    std::cout << "Bye World!\n";
}