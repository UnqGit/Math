#include "Math.hpp"
#include <iostream>

template <typename Arr, typename T>
requires math::matrix::impl::isOneDArr<Arr, T>
void print_1d(const Arr &arr) {
    for (const T &x : arr) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

template <typename T>
void print_matrix(const math::Matrix<T> &matrix) {
    for (size_t i = 0; i < matrix.num_rows(); i++) {
        print_1d<math::matrix::Row<T>, T>(matrix.row(i));
    }
    std::cout << '\n';
}


int main(void) {
    int matrix[4][4] {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {3, 3, 3, 3}
    };
    int matrix2[6] {
        1, 2, 3, 4, 5, 6
    };
    math::Matrix<int> my_matrix(10, 10, math::matrix::ConstructAllocateRule::possible_garbage);
    math::Matrix<int> my_matrix2(10, 10, math::matrix::CAR::zero);
    math::Matrix<int> my_matrix3(10, 10, 9);
    math::Matrix<int> my_matrix4(matrix, 4);
    math::Matrix<int> my_matrix5(matrix2, 6, math::matrix::ConstructOrientationRule::off_diagonal);
    print_matrix<int>(my_matrix);
    print_matrix<int>(my_matrix2);
    print_matrix<int>(my_matrix3);
    print_matrix<int>(my_matrix4);
    print_matrix<int>(my_matrix5);
}