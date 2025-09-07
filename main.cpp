#include <iostream>
#include "RectMatrix.hpp"

template <typename T>
void print_matrix(const Matrix::Rect<T> &mt) {
    for (size_t row = 0; row < mt.row_len(); row++) {
        for (size_t col = 0; col < mt.column_len(); col++) {
            std::cout << mt.at(row, col) << ' ';
        }
        std::cout << '\n';
    }
}

int main(void) {
    Matrix::Rect<int> mt(4, 6);
    Matrix::Rect<int> mt2;
    print_matrix(mt);
    mt2 = mt;
    try {
        std::cout << mt.is_rzero(34) << '\n';
    }
    catch(const std::exception &e) {
        std::cout << "Tried to access elements outside the matrix range; err_msg: " << e.what() << '\n';
    }
    print_matrix(mt2);
}