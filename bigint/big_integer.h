#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <string>
#include <functional>
#include <vector>

struct big_integer {
    big_integer();

    big_integer(big_integer const &);

    big_integer(int32_t);

    big_integer(uint32_t);

    explicit big_integer(std::string const &);

    ~big_integer();

    big_integer &operator=(big_integer const &);

    big_integer operator+() const;

    big_integer operator-() const;

    big_integer operator~() const;

    big_integer &operator+=(big_integer const &);

    big_integer &operator-=(big_integer const &);

    big_integer &operator*=(big_integer const &);

    big_integer &operator/=(big_integer const &);

    big_integer &operator%=(big_integer const &);

    big_integer &operator&=(big_integer const &);

    big_integer &operator^=(big_integer const &);

    big_integer &operator|=(big_integer const &);

    big_integer &operator<<=(int);

    big_integer &operator>>=(int);

    big_integer &operator++();

    big_integer operator++(int);

    big_integer &operator--();

    big_integer operator--(int);

    friend bool operator==(big_integer const &, big_integer const &);

    friend bool operator!=(big_integer const &, big_integer const &);

    friend bool operator>(big_integer const &, big_integer const &);

    friend bool operator<(big_integer const &, big_integer const &);

    friend bool operator>=(big_integer const &, big_integer const &);

    friend bool operator<=(big_integer const &, big_integer const &);

    friend std::string to_string(big_integer);

    void swap(big_integer &);

private:
    std::vector<uint32_t> buf;

    big_integer &apply_operation(big_integer const &, std::function<uint32_t(uint32_t, uint32_t)> const &);

    uint32_t div_by_uint32_t(uint32_t divisor);

    static uint32_t get_trial_multiplier(big_integer const &r, big_integer const &d, size_t m, size_t k);

    bool smaller(big_integer const &, size_t, size_t) const;

    void difference(big_integer const &, size_t, size_t);

    static void long_divide(big_integer &, big_integer &, big_integer &, big_integer &);

    static void divide(big_integer, big_integer, big_integer &, big_integer &);

    bool sign() const;

    void change_data(std::vector<uint32_t> &);

    void clear_empty_slots();

    uint32_t const *data() const;

    uint32_t *non_const_data();
};

big_integer operator+(big_integer, big_integer const &);

big_integer operator-(big_integer, big_integer const &);

big_integer operator*(big_integer, big_integer const &);

big_integer operator/(big_integer, big_integer const &);

big_integer operator%(big_integer, big_integer const &);

big_integer operator&(big_integer, big_integer const &);

big_integer operator^(big_integer, big_integer const &);

big_integer operator|(big_integer, big_integer const &);

big_integer operator>>(big_integer, int);

big_integer operator<<(big_integer, int);

#endif
