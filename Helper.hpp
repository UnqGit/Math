#pragma once

#include "headers.hpp"

namespace math::helper
{
    // Equality checking functions.
    template <std::integral T> // Just because it's better to send numbers by value instead of const T&.
    inline bool is_equal(const T a, const T b) noexcept {
        return a == b;
    }

    template <std::floating_point T>
    inline bool is_equal(const T a, const T b) noexcept {
        const T rel_tol = static_cast<T>(4) * std::numeric_limits<T>::epsilon();
        const T abs_tol = std::numeric_limits<T>::denorm_min();
        return (std::fabs(a - b) <= std::max(rel_tol * std::max(std::fabs(a), std::fabs(b)), abs_tol));
    }

    template <typename T>
    requires ((!std::floating_point<T>) && (!std::integral<T>))
    inline bool is_equal(const T &a, const T &b) {
        return a == b;
    }

    // Zero value holder class.
    class ZeroValueHolder {
        private:
            std::unordered_map<std::type_index, std::any> m_vals;
        private:
            ZeroValueHolder() {}; // Private default constructor.
            ZeroValueHolder(const ZeroValueHolder&) = delete;
            ZeroValueHolder& operator=(const ZeroValueHolder&) = delete;
        public:
            static ZeroValueHolder &instance() noexcept {
                static ZeroValueHolder zero_vals;
                return zero_vals;
            }
        public:
            template <typename T>
            void store_of(const T &val) { // Cannot do noexcept because typeid can throw.
                m_vals[std::type_index(typeid(T))] = val;
            }

            template <typename T>
            T get_of() const {
                auto loc = m_vals.find(std::type_index(typeid(T)));
                if (loc != m_vals.end()) {
                    return std::any_cast<T>(loc->second);
                }
                else throw std::logic_error("Cannot provide the zero value of a type that is not already stored in math::helper::zero_vals.\n");
            }
    };
    ZeroValueHolder& zero_vals = ZeroValueHolder::instance();
}