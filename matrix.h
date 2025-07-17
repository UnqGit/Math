#include <iostream>
#include <string>
#include <initializer_list>
#include <stdexcept>

namespace mat {

static constexpr bool OFFSET_INDEX = true;

template <typename T>
class SquareMatrix;

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
      if(n_entries() == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      m_entries = new T[n_entries()]();
    }
    
    // Dimesion constructor with defualt value.
    RectangularMatrix(size_t rows, size_t columns, const T &to_copy): m_rows(rows), m_columns(columns) {
      if(n_entries() == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      m_entries = new T[n_entries()];
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = to_copy;
    }
    
    // Copy constructor.
    RectangularMatrix(const RectangularMatrix &other): m_rows(other.m_rows), m_columns(other.m_columns) {
      m_entries = new T[n_entries()];
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = other.m_entries[i];
    }
    
    // constructor using an initializer_list.
    RectangularMatrix(size_t rows, size_t columns, const std::initializer_list<T> &il): m_rows(rows), m_columns(columns) {
      if(n_entries() == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      if(n_entries() != il.size()) throw std::invalid_argument("While making a matrix from an initializer_list, list and dimensions provided for matrix creation should give the same number of entries.");
      m_entries = new T[n_entries()];
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = *(il.begin()+i);
    }
    
    // Vectors, optionally lateral or longitudinal.
    RectangularMatrix(const std::initializer_list<T> &il, bool single_column = false) {
      if(il.size() == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      m_rows = !single_column ? 1 : il.size();
      m_columns = single_column ? 1 : il.size();
      m_entries = new T[n_entries()];
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = *(il.begin()+i);
    }
    
    RectangularMatrix(const std::initializer_list<std::initializer_list<T>> &matrix) {
      m_rows = matrix.size();
      if(m_rows == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      m_columns = matrix.begin()->size();
      if(m_columns == 0) throw std::invalid_argument("Any dimension of the matrix can't be 0.");
      m_entries = new T[n_entries()];
      size_t index = 0;
      for(const auto &vector: matrix) {
        if(vector.size() != m_columns) throw std::invalid_argument("Number of entries in each row should be equal(construction error: type: initializer_list<initializer_list<>>).");
        for(const auto &entry: vector) m_entries[index++] = entry;
      }
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
    RectangularMatrix& operator=(RectangularMatrix &&other) noexcept {
      swap(other);
      return *this;
    }
    
    RectangularMatrix& operator=(const RectangularMatrix &other) {
      if(order() != other.order()) throw std::invalid_argument("Copy assignment can't be done with different order-ed matrices.");
      if(this == &other) return *this;
      RectangularMatrix temp(other);
      swap(temp);
      return *this;
    }
    
    // No check for sizes unlike operator=(), so the size can change if different sized matrix is used.
    RectangularMatrix& copy(const RectangularMatrix &other) {
      if(this == &other) return *this;
      RectangularMatrix temp(other);
      swap(temp);
      return *this;
    }
    
    // Fill the matrix with a single element copied in the full matrix.
    RectangularMatrix& fill(const T &to_fill) noexcept {
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = to_fill;
      return *this;
    }
    
    // Fill a single row with the copies of T type, not noexcept because at can throw errors.
    RectangularMatrix& fill_row(size_t row, const T &to_fill) {
      for(size_t i = 0; i < m_columns; i++) at(row, i + OFFSET_INDEX) = to_fill;
      return *this;
    }

    // Fill a single column with the copies of T type, not noexcept because at can throw errors.
    RectangularMatrix& fill_column(size_t column, const T &to_fill) {
      for(size_t i = 0; i < m_rows; i++) at(i + OFFSET_INDEX, column) = to_fill;
      return *this;
    }
    
    // Provides the number of rows present.
    size_t n_rows() const noexcept {
      return m_rows;
    }
    
    // Provides the number of columns present.
    size_t n_columns() const noexcept {
      return m_columns;
    }
    
    // Number of entries in the matrix.
    size_t n_entries() const noexcept {
      return m_rows * m_columns;
    }
    
    // Returns the actual order of the matrix, i.e, m x n.
    std::pair<size_t, size_t> order() const noexcept {
      return {m_rows, m_columns};
    }
    
    bool is_horizontal() const noexcept {
      return m_columns > m_rows;
    }
    
    bool is_vertical() const noexcept {
      return m_rows < m_columns;
    }
    
    bool is_square() const noexcept {
      return m_rows == m_columns;
    }
    
    // Index starts from 1, instead of 0 accounting for general representation of matrix elements but can be changed by changing the boolean in this header file.
    T& at(size_t row, size_t column) {
      if(OFFSET_INDEX && row * column == 0) throw std::out_of_range("Index of matrices start at 1,1. Please start with 1, not 0 (or change the offset to false in header file).");
      if(row > m_rows - !OFFSET_INDEX || column > m_columns - !OFFSET_INDEX) throw std::out_of_range("Index used to access the elements can't be greater than the row/column present.");
      return *(m_entries + m_columns * (row - OFFSET_INDEX) + (column - OFFSET_INDEX));
    }
    
    const T& at(size_t row, size_t column) const {
      if(OFFSET_INDEX && row * column == 0) throw std::out_of_range("Index of matrices start at 1,1. Please start with 1, not 0 (or change the offset to false in header file).");
      if(row > m_rows - !OFFSET_INDEX || column > m_columns - !OFFSET_INDEX) throw std::out_of_range("Index used to access the elements can't be greater than the row/column present.");
      return *(m_entries + m_columns * (row - OFFSET_INDEX) + (column - OFFSET_INDEX));
    }
    
    RectangularMatrix operator-() const {
      RectangularMatrix result(*this);
      for(size_t i = 0; i < n_entries(); i++) result.m_entries[i] = -m_entries[i];
      return result;
    }
    
    RectangularMatrix& negate() {
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] = -m_entries[i];
      return *this;
    }
    
    RectangularMatrix& operator+=(const RectangularMatrix &other) {
      if(order() != other.order()) throw std::invalid_argument("Addition can't be done with different order-ed matrices.");
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] += other.m_entries[i];
      return *this;
    }
    
    RectangularMatrix& operator-=(const RectangularMatrix &other) {
      if(order() != other.order()) throw std::invalid_argument("Subtraction can't be done with different order-ed matrices.");
      for(size_t i = 0; i < n_entries(); i++) m_entries[i] -= other.m_entries[i];
      return *this;
    }
    
    RectangularMatrix operator*(const RectangularMatrix &other) const {
      if(m_columns != other.m_rows) throw std::invalid_argument("Multiplication can't be done when the number of columns in first matrix is different than number of rows in second matrix.");
      RectangularMatrix result(m_rows, other.m_columns);
      for(size_t r = 0; r < m_rows; r++)
        for(size_t c = 0; c < other.m_columns; c++)
          for (int i = 0; i < m_columns; i++)
            result.at(r + OFFSET_INDEX, c + OFFSET_INDEX) += at(r + OFFSET_INDEX, i + OFFSET_INDEX) * other.at(i + OFFSET_INDEX, c + OFFSET_INDEX);
      return result;
    }
    
    RectangularMatrix operator+(const RectangularMatrix &other) const {
      RectangularMatrix result(*this);
      result += other;
      return result;
    }
    
    RectangularMatrix operator-(const RectangularMatrix &other) const {
      RectangularMatrix result(*this);
      result -= other;
      return result;
    }
    
    RectangularMatrix& operator*=(const RectangularMatrix &other) {
      RectangularMatrix result = (*this)*other;
      swap(result);
      return *this;
    }
    
    bool operator==(const RectangularMatrix &other) {
      if(this == &other) return true;
      if(order() != other.order()) return false;
      for(size_t i = 0; i < order(); i++) if(m_entries[i] != other.m_entries[i]) return false;
      return true;
    }
    
    bool operator!=(const RectangularMatrix &other) const noexcept {
      return !((*this) == other);
    }
    
    // Mirror about x-axis.
    RectangularMatrix flip_h() const {
      if(m_rows == 1) return *this;
      RectangularMatrix result(*this);
      for(size_t r = 0; r < m_rows / 2; r++)
        for(size_t c = 0; c < m_columns; c++)
          std::swap(result.m_entries[r*m_columns + c], result.m_entries[(m_rows - 1 - r)*m_columns + c]);
      return result;
    }
    
    // Mirror about y-axis.
    RectangularMatrix flip_v() const {
      if(m_columns == 1) return *this;
      RectangularMatrix result(*this);
      for(size_t c = 0; c < m_columns / 2; c++)
        for(size_t r = 0; r < m_rows; r++)
          std::swap(result.m_entries[r*m_columns + c], result.m_entries[r*m_columns + (m_columns - 1 - c)]);
      return result;
    }
    
    // Transpose of a matrix => a21(new) = a12(old).
    RectangularMatrix transpose() const {
      // Early returns.
      if(n_entries() == 1) return *this;
      if(m_rows == 1 || m_columns == 1) {
        RectangularMatrix result(*this);
        std::swap(result.m_rows, result.m_columns);
        return result;
      }
      // Main work.
      RectangularMatrix result(m_columns,m_rows); // Potentially slower but safer...let's just stay in our lane, shall we?
      for(size_t r = 0; r < m_rows; r++)
        for(size_t c = 0; c < m_columns; c++)
          result.at(c + OFFSET_INDEX, r + OFFSET_INDEX) = at(r + OFFSET_INDEX, c + OFFSET_INDEX);
      return result;
    }
    
    // Rotate the matrix clockwise.
    RectangularMatrix rotate_cw() const {
      // Early returns.
      if(n_entries() == 1) return *this;
      if(m_columns == 1) {
        RectangularMatrix result(*this);
        std::swap(result.m_columns, result.m_rows);
        for(size_t i = 0; i < n_entries() / 2; i++) std::swap(result.m_entries[i], result.m_entries[m_rows - 1 - i]); // Reverse the order.
        return result;
      }
      if(m_rows == 1) {
        RectangularMatrix result(*this);
        std::swap(result.m_columns, result.m_rows);
        return result;
      }
      // Main work.
      RectangularMatrix result(m_columns, m_rows);
      for(size_t r = 0; r < result.m_rows; r++)
        for(size_t c = 0; c < result.m_columns; c++)
          result.at(r + OFFSET_INDEX, c + OFFSET_INDEX) = at(m_rows - 1 - c + OFFSET_INDEX, r + OFFSET_INDEX);
      return result;
    }
    
    void print(std::ostream &os) const {
      os << "\n{\n";
      for(size_t r = 0; r < m_rows; r++)
        for(size_t c = 0; c < m_columns; c++)
          os << at(r + OFFSET_INDEX, c + OFFSET_INDEX) << (c < m_columns - 1 ? ", " : (r < m_rows - 1 ? "\n" : "\n}"));
    }
    friend std::ostream& operator<<(std::ostream &os, const RectangularMatrix &matrix) {
      matrix.print(os);
      return os;
    }
    
    // Destructor....RAII.
    ~RectangularMatrix() {
      delete[] m_entries;
    }
    
  private:
    operator SquareMatrix<T>() const;
    friend class SquareMatrix<T>;
};

// To make swappable using std::swap.
template<typename T>
void swap(RectangularMatrix<T> &lfs, RectangularMatrix<T> &rhs) noexcept {
  lfs.swap(rhs);
}

}

int main() {
  mat::RectangularMatrix<int> my_matrix({{1,2},{1,1}});
  // my_matrix.print(std::cout);
  std::cout << my_matrix << "\n\n";
  std::cout << my_matrix.rotate_cw();
  return 0;
}
