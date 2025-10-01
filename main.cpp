#include "Math.hpp"
#include <iostream>

int main(void) {
    int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    math::Matrix<int> my_matrix(data, 10, math::matrix::ConstructOrientationRule::main_diagonal);
    std::cout << my_matrix.order().row() << ' ' << my_matrix.order().column() << std::endl;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) 
            std::cout << my_matrix.at(i, j) << ' ';
        std::cout << '\n';
    }
}