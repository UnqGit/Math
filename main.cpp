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

int main(void) {
    using namespace math;
    using namespace math::matrix;
    std::cout << "Hello World!\n\n";
    std::initializer_list<Matrix<int>> MatrixArr {
        Matrix<int>(), Matrix<int>(Order(1, 2)), Matrix<int>(3), Matrix<int>(3, 5, CSR::right_half), Matrix<int>(Order(3, 2), CAR::possible_garbage), Matrix<int>(Order(3, 2), 4)
    };
    // std::for_each(MatrixArr.begin(), MatrixArr.end(), [](const Matrix<int> &mat) {
    //     print_matrix<int>(mat);
    // });
    std::cout << "Bye World!\n";
}