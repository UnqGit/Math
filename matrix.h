#include <iostream>
#include <string>

// Matrces don't usually grow in size, so it will not be a wrapper on std::vector.
template <typename T>
class RectangularMatrix {
  protected:
    T* __entries = nullptr;
    size_t __rows = 0;
    size_t __columns = 0;
  
  public:
    RectangularMatrix(size_t rows, size_t columns): __rows(rows), __columns(columns) {
      __entries = new T[rows * columns]();
    }
    
    RectangularMatrix(size_t rows, size_t columns, const T &to_copy): __rows(rows), __columns(columns) {
      size_t size = rows * columns;
      __entries = new T[size]();
      for(int i = 0; i < size; i++) __entries[i] = to_copy;
    }
    
    RectangularMatrix(const RectangularMatrix &other): __rows(other.__rows), __columns(other.__columns) {
       
    }
    
    RectangularMatrix& operator=(const RectangularMatrix& other) {
      if(this->__rows != other.__rows || this->__columns != other.__columns) throw std::runtime_error("Copy assignment can't be done with different order-ed matrices.");
      if(this == &other) return *this;
      
      return *this;
    }
    
    // Index starts from 1, instead of 0 accounting for general representation of matrix elements.
    T& at(size_t row, size_t column) {
      if(row * column == 0) throw std::runtime_error("Index of matrices start at 1,1. Please start with 1, not 0.");
      if(row > __rows || column > __columns) throw std::runtime_error("Index used to access the elements can't be greater than the row/column present.");
      return *(__entries + __columns * (row - 1) + (column - 1));
    }
    
    ~RectangularMatrix() {
      delete[] __entries;
    }
};

int main() {
  RectangularMatrix<std::string> my_matrix(2,1,"Hello");
  std::cout << my_matrix.at(2,1) << std::endl;
  my_matrix.at(1,1) = "Woah";
  std::cout << my_matrix.at(1,1);
  return 0;
}
