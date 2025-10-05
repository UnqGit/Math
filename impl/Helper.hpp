// Helper.hpp
#pragma once

#include "Headers.hpp"

namespace math
{
    template <typename T>
    concept ProperEquality = requires(const T &a, const T &b) {
        { a == b } -> std::same_as<bool>;
    };

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
    requires (((!std::floating_point<T>) && (!std::integral<T>)) && ProperEquality<T>)
    inline bool is_equal(const T &a, const T &b) {
        return a == b;
    }

    template <typename T>
    concept isEqualityOperationPossible = requires(const T &a, const T &b) {
        { math::is_equal(a, b) } -> std::same_as<bool>;
    };

    template <typename T>
    concept isAdditionPossible = requires(T a, T b) {
        { a + b } -> std::same_as<T>;
        requires std::same_as<std::remove_reference_t<decltype(a += b)>, T>;
    };

    template <typename T>
    concept isSubtractionPossible = requires(T a, T b) {
        { a - b } -> std::same_as<T>;
        requires std::same_as<std::remove_reference_t<decltype(a -= b)>, T>;
    };

    template <typename T>
    concept isMultiplicationPossible = requires(T a, T b) {
        { a * b } -> std::same_as<T>;
        requires std::same_as<std::remove_reference_t<decltype(a *= b)>, T>;
    };

    // Zero value holder class.
    class ZeroValueHolder {
        private:
            std::unordered_map<std::type_index, std::any> m_vals;
        private:
            ZeroValueHolder() { // Helping the file user by pre-saving some of the types' 0 val.
                store_of<char>(static_cast<char>(0));
                store_of<short>(static_cast<short>(0));
                store_of<int>(static_cast<int>(0));
                store_of<long>(static_cast<long>(0));
                store_of<long long>(static_cast<long long>(0));
                store_of<unsigned char>(static_cast<unsigned char>(0));
                store_of<unsigned short>(static_cast<unsigned short>(0));
                store_of<unsigned int>(static_cast<unsigned int>(0));
                store_of<unsigned long>(static_cast<unsigned long>(0));
                store_of<unsigned long long>(static_cast<unsigned long long>(0));
                store_of<const char>(static_cast<const char>(0));
                store_of<const short>(static_cast<const short>(0));
                store_of<const int>(static_cast<const int>(0));
                store_of<const long>(static_cast<const long>(0));
                store_of<const long long>(static_cast<const long long>(0));
                store_of<const unsigned char>(static_cast<const unsigned char>(0));
                store_of<const unsigned short>(static_cast<const unsigned short>(0));
                store_of<const unsigned int>(static_cast<const unsigned int>(0));
                store_of<const unsigned long>(static_cast<const unsigned long>(0));
                store_of<const unsigned long long>(static_cast<const unsigned long long>(0));
                store_of<float>(static_cast<float>(0));
                store_of<double>(static_cast<double>(0));
                store_of<long double>(static_cast<long double>(0));
                store_of<const float>(static_cast<const float>(0));
                store_of<const double>(static_cast<const double>(0));
                store_of<const long double>(static_cast<const long double>(0));
                
                store_of<char*>(nullptr);
                store_of<short*>(nullptr);
                store_of<int*>(nullptr);
                store_of<long*>(nullptr);
                store_of<long long*>(nullptr);
                store_of<unsigned char*>(nullptr);
                store_of<unsigned short*>(nullptr);
                store_of<unsigned int*>(nullptr);
                store_of<unsigned long*>(nullptr);
                store_of<unsigned long long*>(nullptr);
                store_of<const char*>(nullptr);
                store_of<const short*>(nullptr);
                store_of<const int*>(nullptr);
                store_of<const long*>(nullptr);
                store_of<const long long*>(nullptr);
                store_of<const unsigned char*>(nullptr);
                store_of<const unsigned short*>(nullptr);
                store_of<const unsigned int*>(nullptr);
                store_of<const unsigned long*>(nullptr);
                store_of<const unsigned long long*>(nullptr);
                store_of<float*>(nullptr);
                store_of<double*>(nullptr);
                store_of<long double*>(nullptr);
                store_of<const float*>(nullptr);
                store_of<const double*>(nullptr);
                store_of<const long double*>(nullptr);

                store_of<void*>(nullptr);
                store_of<const void*>(nullptr);
            }; // Private default constructor.
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
            bool exists_of() {
                auto loc = m_vals.find(std::type_index(typeid(T)));
                return (loc != m_vals.end());   
            }

            template <typename T>
            const T &get_of() const {
                const auto loc = m_vals.find(std::type_index(typeid(T)));
                if (loc != m_vals.end()) return std::any_cast<const T&>(loc->second);
                else throw std::logic_error("Cannot provide the zero value of a type that is not already stored in math::helper::zero_vals.\n");
            }
    };
    ZeroValueHolder& zero_vals = ZeroValueHolder::instance();
}

namespace math::helper
{
    
    template <typename Container, typename RequiredData>
    concept isOneDArr =
        std::ranges::sized_range<Container> &&
        std::same_as<
            std::decay_t<std::ranges::range_value_t<Container>>,
            std::decay_t<RequiredData>
        >;

    template <typename Container, typename RequiredData>
    concept isTwoDArr =
        std::ranges::sized_range<Container> &&
        isOneDArr<
            std::decay_t<std::ranges::range_value_t<Container>>,
            RequiredData
        >;

}