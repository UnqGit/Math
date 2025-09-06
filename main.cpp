#include <iostream>
#include "RectMatrix.hpp"
#include <vector>
#include <chrono>

int main(void) {
    int z = 0;
    while(z++ < 30) {
        int m, n;
        std::cout << "Enter rows and columns: ";
        std::cin >> m >> n;
        Matrix::Rect<int> rectMatrix(Matrix::Order(m, n), 4);

        auto start = std::chrono::high_resolution_clock::now();
        decltype(start) stop;
        long long iterations = 1e4;
        std::vector<double> times(iterations);

        for (long long i = 0; i < iterations; i++) {
            for (long long j = 0; j < iterations; j++) {
                Matrix::Rect copy(rectMatrix);
                start = std::chrono::high_resolution_clock::now();
                copy += copy;
                stop = std::chrono::high_resolution_clock::now();
                times[i] += std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
            }
            times[i] /= iterations;
        }

        std::sort(times.begin(), times.end());
        std::cout << "For m and n of: " << m << ' ' << n << '\n';
        std::cout << "The minimum time taken was: " << times.front() << "ns.\n";
        std::cout << "The median time taken was: " << times.at(iterations / 2) << "ns.\n";
        std::cout << "The maximum time taken was: " << times.back() << "ns.\n\n";
    }
}