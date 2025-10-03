#include "Math.hpp"
#include <iostream>

template <typename Arr, typename T>
requires math::helper::isOneDArr<Arr, T>
void print_1d(const Arr &arr) {
    for (const T &x : arr) std::cout << x << ' ';
    std::cout << '\n';
}

template <typename T>
void print_matrix(const math::Matrix<T> &matrix) {
    for (size_t i = 0; i < matrix.num_rows(); i++) print_1d<math::matrix::Row<T>, T>(matrix.row(i));
    std::cout << '\n';
}

int main(void) {
    std::cout << "Hello World!\n";
    int matrix[6] {
        1, 2, 3, 4, 5, 6
    };
    math::Matrix<int> myMatrix(matrix, 6);
    math::Matrix<int> myMatrix2(matrix, 6, math::matrix::COR::vertical);
    print_matrix<int> (myMatrix);
    print_matrix<int> (myMatrix2);
    print_matrix<int> (myMatrix * myMatrix2);
    print_matrix<int> (myMatrix2 * myMatrix);
    print_matrix<int> (myMatrix + myMatrix2);
    std::cout << "Bye World!\n";
}