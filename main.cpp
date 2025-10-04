#include <iostream>
#include "Math.hpp"

template <typename Arr, typename T>
requires math::helper::isTwoDArr<Arr, T>
void print_two_d_arr(const Arr &arr) noexcept {
    for (const auto &row : arr) {
        for (const auto &elem : row) std::cout << elem << ' ';
        std::cout << '\n';
    }
    std::cout << '\n';
}
template <typename T>
void print_matrix(const math::Matrix<T> &mat) noexcept {
    print_two_d_arr<math::Matrix<T>, T>(mat);
}

// template <typename T>
// void print_matrix(const math::Matrix<T> &mat) noexcept {
//     for (auto it = mat.begin_one_d(); it != mat.end_one_d(); ++it) {
//         std::cout << *it << ' ';
//     }
//     std::cout << '\n';
// }


// template <typename T>
// void print_matrix(const math::Matrix<T> &mat) noexcept {
//     for (size_t i = 0; i < mat.num_rows(); i++) {
//         for (const auto &x : mat[i]) std::cout << x << ' ';
//         std::cout << '\n';
//     }
//     std::cout << '\n';
// }

int main(void) {
    // static_assert(std::input_iterator<math::matrix::MatrixIterator<int>>);
    // static_assert(math::helper::isOneDArr<math::matrix::Row<int>, int>);
    // static_assert(math::helper::isOneDArr<const math::matrix::Row<int>, int>);
    std::cout << "Hello World!\n";
    int matrix[6] {
        1, 2, 3, 4, 5, 6
    };
    math::Matrix<int> myMatrix(matrix, 6, math::matrix::COR::horizontal);
    math::Matrix<int> myMatrix2(matrix, 6, math::matrix::COR::vertical);
    math::Matrix<int> multMatrix = (myMatrix2 * myMatrix);
    print_matrix<int> (multMatrix);
    std::sort(multMatrix.begin_one_d(), multMatrix.end_one_d());
    print_matrix<int> (multMatrix);
    print_matrix<int> (multMatrix.transpose());
    std::cout << "Bye World!\n";
}