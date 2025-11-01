// VariableString.hpp
#pragma once

#include "..\..\Helper\Headers.hpp"

namespace math::expr {

using size8_t = unsigned char;
constexpr size_t SSO_SIZE = 2 * sizeof(mut_ptr<char>) - 1; // So that the size of the VariableString is at most as big as the size of two pointers.
constexpr size8_t MAX_VAR_SIZE = ~0;
constexpr size8_t MIN_VAR_SIZE = 1;

struct VariableStringData {
    #pragma pack(1)
    using memloc_t = union memory_location
    {
        char internal_buffer[SSO_SIZE];
        mut_ptr<char> external_buffer;
    };
    #pragma pack()
    memloc_t data;
    size8_t size;
};

class VariableString {
public:
    using data_t = VariableStringData;

public:
    constexpr VariableString() noexcept {
        m_data.size = 1;
        m_data.data.internal_buffer[0] = 'x';
        m_data.data.internal_buffer[1] = '\0';
    }

    VariableString(const char val) {
        m_data.size = 1;
        m_data.data.internal_buffer[0] = val;
        m_data.data.internal_buffer[1] = '\0';
    }

    VariableString(const char val, size_t size) {
        if (size < MIN_VAR_SIZE) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > MAX_VAR_SIZE) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        m_data.size = size;
        if (m_data.size >= SSO_SIZE) m_data.data.external_buffer = static_cast<mut_ptr<char> >(::operator new(static_cast<size_t>(m_data.size + 1)));
        mut_ptr<char> loc = m_data.size >= SSO_SIZE ? m_data.data.external_buffer : m_data.data.internal_buffer;
        std::memset(loc, val, m_data.size);
        loc[m_data.size] = '\0';
    }

    VariableString(read_ptr<char> data, size_t size) {
        if (size < MIN_VAR_SIZE) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > MAX_VAR_SIZE) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        m_data.size = size;
        if (m_data.size >= SSO_SIZE) m_data.data.external_buffer = static_cast<mut_ptr<char> >(::operator new(static_cast<size_t>(m_data.size + 1)));
        mut_ptr<char> loc = m_data.size >= SSO_SIZE ? m_data.data.external_buffer : m_data.data.internal_buffer;
        std::memcpy(loc, data, m_data.size);
        loc[m_data.size] = '\0';
    }

    VariableString(std::string_view s) : VariableString(s.data(), s.size()) {}

public:
    VariableString(const VariableString &other) : VariableString(other.data(), other.size()) {}
    VariableString &operator=(const VariableString &other) {
        if (this != &other) {
            VariableString temp(other);
            this->swap(temp);
        }
        return *this;
    }
    VariableString(VariableString &&other) noexcept {
        m_data.size = 0;
        this->swap(other);
    }
    VariableString &operator=(VariableString &&other) noexcept {
        if (this != &other) this->swap(other);
        return *this;
    }

public:
    ~VariableString() noexcept {
        if (m_data.size >= SSO_SIZE) ::operator delete(m_data.data.external_buffer);
    }

public:
    void swap(VariableString &other) noexcept {
        data_t temp = other.m_data;
        other.m_data = m_data;
        m_data = temp;
    }

public:
    const_ptr<char> c_str() const noexcept {
        return data();
    }
    const_ptr<char> data() const noexcept {
        return m_data.size < SSO_SIZE ? m_data.data.internal_buffer : m_data.data.external_buffer;
    }
    size8_t size() const noexcept {
        return m_data.size;
    }
    constexpr bool empty() const noexcept {
        return false;
    }

public:
    const char &operator[](const size_t index) const noexcept {
        return data()[index];
    }
    const char &at(const size_t index) const {
        if (index >= m_data.size) throw std::out_of_range("Cannot access index out of the size of this variable's name.");
        return data()[index];
    }

public:
    const_ptr<char> begin() const noexcept {
        return data();
    }
    const_ptr<char> end() const noexcept {
        return data() + m_data.size;
    }

public:
    bool operator==(const VariableString &other) noexcept {
        if (this == &other) return true;
        if (m_data.size != other.m_data.size) return false;
        return std::memcmp(data(), other.data(), m_data.size) == 0;
    }
    bool operator!=(const VariableString &other) noexcept {
        if (this == &other) return false;
        if (m_data.size != other.m_data.size) return true;
        return std::memcmp(data(), other.data(), m_data.size) != 0;
    }

public:
    operator std::string_view() const {
        return std::string_view(data(), m_data.size);
    }

private:
    data_t m_data;
};
using VariableName = VariableString;

VariableString operator"" _var(read_ptr<char> data, size_t size) {
    return VariableString(data, size);
}

}