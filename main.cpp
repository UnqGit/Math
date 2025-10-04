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
//     std::for_each(mat.begin_one_d(), mat.end_one_d(), [](const T &x){ std::cout << x << ' '; });
//     std::cout << "\n\n";
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
    std::cout << "Hello World!\n\n";
    int matrix[6] {
        1, 2, 3, 4, 5, 6
    };
    math::Matrix<int> myMatrix(matrix, 6, math::matrix::COR::horizontal);
    math::Matrix<int> myMatrix2(matrix, 6, math::matrix::COR::vertical);
    math::Matrix<int> multMatrix = (myMatrix2 * myMatrix);
    print_matrix<int> (myMatrix);
    print_matrix<int> (myMatrix2);
    print_matrix<int> (multMatrix);
    std::sort(multMatrix.begin_one_d(), multMatrix.end_one_d());
    print_matrix<int> (multMatrix);
    print_matrix<int> (multMatrix.transpose_in_place());
    math::Matrix<int> myMatrix3(std::initializer_list<std::initializer_list<int>>
    {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}        
    });
    print_matrix<int> (myMatrix3);
    std::cout << myMatrix.is_null() << ' ' << myMatrix2.is_null() << ' ' << myMatrix3.is_null() << '\n';
    std::cout << "Count of 0 in m3: " << myMatrix3.count(0) << '\n';
    std::cout << myMatrix3.are_all_same() << '\n';
    print_matrix<int> (myMatrix += myMatrix);
    std::cout << (math::Matrix<int>(matrix, 6) == math::Matrix<int>(matrix, 6)) << "\n\n";
    std::cout << "Bye World!\n";
}