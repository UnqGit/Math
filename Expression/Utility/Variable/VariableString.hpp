// VariableString.hpp
#pragma once

#include "Utility\VariableNameCheck.hpp"

namespace math::expr::var {

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
        auto validity = is_valid_var_name(val);
        if (validity == VNET::one_char_var_name_be_letter) throw std::invalid_argument("Name of a variable of size one must be an english alphabet.");
        else if (validity == VNET::is_math_constant) throw std::invalid_argument("Name of the variable can't be a math constant (e, pi)");
        m_data.size = 1;
        m_data.data.internal_buffer[0] = val;
        m_data.data.internal_buffer[1] = '\0';
    }

    VariableString(const char val, size_t size) {
        if (size < MIN_VAR_SIZE) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > MAX_VAR_SIZE) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        if (is_valid_var_name(val) == VNET::first_character_be_letter) throw std::invalid_argument("Name of the variable must start with an english alphabet.");
        m_data.size = size;
        if (m_data.size >= SSO_SIZE) m_data.data.external_buffer = static_cast<mut_ptr<char> >(::operator new(static_cast<size_t>(m_data.size + 1)));
        mut_ptr<char> loc = m_data.size >= SSO_SIZE ? m_data.data.external_buffer : m_data.data.internal_buffer;
        std::memset(loc, val, m_data.size);
        loc[m_data.size] = '\0';
    }

    VariableString(read_ptr<char> data, size_t size) {
        if (size < MIN_VAR_SIZE) throw std::invalid_argument("Can't instantiate a variable with no name.");
        if (size > MAX_VAR_SIZE) throw std::invalid_argument("Can't make a variable with a name greater than 255 characters.");
        auto validity = is_valid_var_name(data, size);
        switch(validity) {
            case VNET::no_err: break;
            case VNET::one_char_var_name_be_letter:
            case VNET::first_character_be_letter: throw std::invalid_argument("Name of the variable must start with an english alphabet.");
            case VNET::contains_space: throw std::invalid_argument("Name of a variable can't contain a whitespace.");
            case VNET::has_invalid_char: throw std::invalid_argument("Name of the variable can't contain a character that is not supported(!alphabet, !number, !'_').");
            case VNET::is_math_constant: throw std::invalid_argument("Name of the variable can't be a math constant (e, pi)");
            case VNET::is_math_function: throw std::invalid_argument("Name of the variable can't be a math function (sin, cos, etc.).");
            case VNET::last_letter_underscore: throw std::invalid_argument("Last letter in the name of a variable can't be an underscore.");
            case VNET::underscore_must_separate_digits: throw std::invalid_argument("Digits in the name of a variable must be separated by an underscore.");
            default: break;
        }
        m_data.size = size;
        if (m_data.size >= SSO_SIZE) m_data.data.external_buffer = static_cast<mut_ptr<char> >(::operator new(static_cast<size_t>(m_data.size + 1)));
        mut_ptr<char> loc = m_data.size >= SSO_SIZE ? m_data.data.external_buffer : m_data.data.internal_buffer;
        std::memcpy(loc, data, m_data.size);
        loc[m_data.size] = '\0';
    }

    VariableString(std::string_view s) : VariableString(s.data(), s.size()) {}
    
    template <size_t N>
    VariableString(const char (&data)[N]) requires (N != 1 && N < 256) : VariableString(data, N - 1) {}

public:
    VariableString(const VariableString &other) {
        if (other.size() < SSO_SIZE) {
            m_data = other.m_data;
        }
        else {
            m_data.size = other.size();
            m_data.data.external_buffer = static_cast<mut_ptr<char> >(::operator new(m_data.size + 1));
            std::memcpy(m_data.data.external_buffer, other.m_data.data.external_buffer, m_data.size);
        }
    }
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

}

namespace math::literals {

math::expr::var::VariableString operator""_var(read_ptr<char> data, size_t size) {
    return math::expr::var::VariableString(data, size);
}

}