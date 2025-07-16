#include <iostream>
#include <string>

namespace mat {

static constexpr bool OFFSET_INDEX = true;

// Matrces don't usually grow in size, so it will not be a wrapper on std::vector.
template <typename T>
class RectangularMatrix {
  protected:
    T* m_entries = nullptr;
    size_t m_rows = 0;
    size_t m_columns = 0;
  
  public:
    // Dimension constructors.
    RectangularMatrix(size_t rows, size_t columns): m_rows(rows), m_columns(columns) {
      if(order() == 0) throw std::runtime_error("Any dimension of the matrix can't be 0.");
      m_entries = new T[order()]();
    }
    
    // Dimesion constructor with defualt value.
    RectangularMatrix(size_t rows, size_t columns, const T &to_copy): m_rows(rows), m_columns(columns) {
      if(order() == 0) throw std::runtime_error("Any dimension of the matrix can't be 0.");
      m_entries = new T[order()];
      for(size_t i = 0; i < order(); i++) m_entries[i] = to_copy;
    }
    
    // Copy constructor.
    RectangularMatrix(const RectangularMatrix &other): m_rows(other.m_rows), m_columns(other.m_columns) {
      m_entries = new T[order()];
      for(size_t i = 0; i < order(); i++) m_entries[i] = other.m_entries[i];
    }
    
    // Move constructor.
    RectangularMatrix(RectangularMatrix &&other): m_rows(other.m_rows), m_columns(other.m_columns), m_entries(other.m_entries) {
      other.m_rows = 0; // Convert other to defualt.
      other.m_columns = 0;
      other.m_entries = nullptr;
    }
    
    RectangularMatrix& swap(RectangularMatrix &other) noexcept {
      std::swap(m_rows, other.m_rows);
      std::swap(m_columns, other.m_columns);
      std::swap(m_entries, other.m_entries);
      return *this;
    }
    
    // Move assignment operator.
    RectangularMatrix& operator=(RectangularMatrix &&other) {
      if(this != &other) swap(other);
      return *this;
    }
    
    RectangularMatrix& operator=(const RectangularMatrix &other) {
      if(this->m_rows != other.m_rows || this->m_columns != other.m_columns) throw std::runtime_error("Copy assignment can't be done with different order-ed matrices.");
      if(this == &other) return *this;
      RectangularMatrix temp(other);
      swap(temp);
      return *this;
    }
    
    // No check for sizes unlike operator=(), so the size can change if different sized matrix is used.
    RectangularMatrix& copy(const RectangularMatrix& other) {
      if(this == &other) return *this;
      RectangularMatrix temp(other);
      swap(temp);
      return *this;
    }
    
    size_t order() const noexcept {
      return m_rows * m_columns;
    }
    
    // Index starts from 1, instead of 0 accounting for general representation of matrix elements but can be changed by changing the boolean in this header file.
    T& at(size_t row, size_t column) {
      if(OFFSET_INDEX && row * column == 0) throw std::runtime_error("Index of matrices start at 1,1. Please start with 1, not 0 (or change the offset to false in header file).");
      if(row > m_rows + OFFSET_INDEX || column > m_columns + OFFSET_INDEX) throw std::runtime_error("Index used to access the elements can't be greater than the row/column present.");
      return *(m_entries + m_columns * (row - OFFSET_INDEX) + (column - OFFSET_INDEX));
    }
    
    const T& at(size_t row, size_t column) const {
      if(OFFSET_INDEX && row * column == 0) throw std::runtime_error("Index of matrices start at 1,1. Please start with 1, not 0 (or change the offset to false in header file).");
      if(row > m_rows + OFFSET_INDEX || column > m_columns + OFFSET_INDEX) throw std::runtime_error("Index used to access the elements can't be greater than the row/column present.");
      return *(m_entries + m_columns * (row - OFFSET_INDEX) + (column - OFFSET_INDEX));
    }
    
    // Destructor....RAII.
    ~RectangularMatrix() {
      delete[] m_entries;
    }
};

// To make swappable using std::swap.
template<typename T>
void swap(RectangularMatrix<T> &lfs, RectangularMatrix<T> &rhs) noexcept {
  lfs.swap(rhs);
}

}

int main() {
  RectangularMatrix<std::string> my_matrix(2,1,"Hello");
  std::cout << my_matrix.at(2,1) << std::endl;
  my_matrix.at(1,1) = "Woah";
  std::cout << my_matrix.at(1,1);
  return 0;
}
