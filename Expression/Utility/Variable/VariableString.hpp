// VariableString.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

namespace math::expr {

using size8_t = unsigned char;

class VariableString {
public:
    VariableString(const char val) : m_size(1) {
        m_data = static_cast<mut_ptr<char> >(::operator new(m_size + 1));
        m_data[0] = val;
        m_data[1] = '\0';
    }

    VariableString(const char val, size_t size) : m_size(size) {
        if (size == 0) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > 255) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        m_data = static_cast<mut_ptr<char> >(::operator new(m_size + 1));
        std::memset(m_data, val, m_size);
        m_data[m_size] = '\0';
    }

    VariableString(read_ptr<char> data, size_t size) : m_size(size) {
        if (size == 0) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > 255) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        m_data = static_cast<mut_ptr<char> >(::operator new(m_size + 1));
        std::memcpy(m_data, data, m_size);
        m_data[m_size] = '\0';
    }

    VariableString(const std::string_view &s) : VariableString(s.data(), s.size()) {}

public:
    VariableString(const VariableString &other) : VariableString(other.m_data, other.m_size) {};
    VariableString &operator=(const VariableString &other) {
        if (this != &other) {
            VariableString temp(other);
            this->swap(temp);
        }
        return *this;
    }
    VariableString(VariableString &&other) noexcept : m_data(nullptr), m_size(0) {
        this->swap(other);
    }
    VariableString &operator=(VariableString &&other) noexcept {
        if (this != &other) this->swap(other);
        return *this;
    }

public:
    ~VariableString() noexcept {
        ::operator delete(m_data);
    }

public:
    void swap(VariableString &other) noexcept {
        mut_ptr<char> temp = m_data;
        m_data = other.m_data;
        other.m_data = temp;
        size8_t temp_size = m_size;
        m_size = other.m_size;
        other.m_size = temp_size;
    }

public:
    read_ptr<char> c_str() const noexcept {
        return m_data;
    }
    read_ptr<char> data() const noexcept {
        return m_data;
    }
    size8_t size() const noexcept {
        return m_size;
    }
    constexpr bool empty() const noexcept {
        return false;
    }

public:
    const char &operator[](const size_t index) const noexcept {
        return m_data[index];
    }
    const char &at(const size_t index) const {
        if (index >= m_size) throw std::out_of_range("Cannot access index out of the size of this variable's name.");
        return m_data[index];
    }

public:
    const_ptr<char> begin() const noexcept {
        return m_data;
    }
    const_ptr<char> end() const noexcept {
        return m_data + m_size;
    }

public:
    bool operator==(const VariableString &other) noexcept {
        if (this == &other) return true;
        if (m_size != other.m_size) return false;
        return std::memcmp(m_data, other.m_data, m_size) == 0;
    }
    bool operator!=(const VariableString &other) noexcept {
        if (this == &other) return false;
        if (m_size != other.m_size) return true;
        return std::memcmp(m_data, other.m_data, m_size) != 0;
    }

public:
    operator std::string_view() const {
        return std::string_view(m_data, m_size);
    }

private:
    mut_ptr<char> m_data;
    size8_t m_size;
};
using VariableName = VariableString;

}