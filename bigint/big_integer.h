#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <string>
#include <functional>

struct big_integer {
    big_integer();
    big_integer(big_integer const&);
    big_integer(int);
    big_integer(uint32_t);
    explicit big_integer(std::string const&);
    ~big_integer();

    big_integer& operator=(big_integer const&);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator+=(big_integer const&);
    big_integer& operator-=(big_integer const&);
    big_integer& operator*=(big_integer);
    big_integer& operator/=(big_integer const&);
    big_integer& operator%=(big_integer const&);

    big_integer& operator&=(big_integer const&);
    big_integer& operator^=(big_integer const&);
    big_integer& operator|=(big_integer const&);

    big_integer& operator<<=(int);
    big_integer& operator>>=(int);

    big_integer& operator++();
    big_integer const operator++(int);

    big_integer& operator--();
    big_integer const operator--(int);

    friend bool operator==(big_integer const&, big_integer const&);
    friend bool operator!=(big_integer const&, big_integer const&);
    friend bool operator>(big_integer const&, big_integer const&);
    friend bool operator<(big_integer const&, big_integer const&);
    friend bool operator>=(big_integer const&, big_integer const&);
    friend bool operator<=(big_integer const&, big_integer const&);

    friend std::string to_string(big_integer);

    void swap(big_integer&);

private:
    uint32_t *data_;
    size_t size_;

    big_integer& apply_operation(big_integer const&, std::function<uint32_t(uint32_t, uint32_t)> const&);

    uint32_t small_div(uint32_t);
    static uint32_t trial(big_integer const&, big_integer const&, uint32_t, uint32_t);
    bool smaller(big_integer const&, uint32_t, uint32_t) const;
    void difference(big_integer const&, uint32_t, uint32_t);
    static size_t long_divide(big_integer&, big_integer&, uint32_t* &, big_integer&);
    static size_t divide(big_integer, big_integer, uint32_t* &, big_integer&);

    bool sign() const;

    void change_data(size_t, uint32_t*);
    uint32_t *data() const;
};
big_integer operator+(big_integer, big_integer const&);
big_integer operator-(big_integer, big_integer const&);
big_integer operator*(big_integer, big_integer const&);
big_integer operator/(big_integer, big_integer const&);
big_integer operator%(big_integer, big_integer const&);

big_integer operator&(big_integer, big_integer const&);
big_integer operator^(big_integer, big_integer const&);
big_integer operator|(big_integer, big_integer const&);

big_integer operator>>(big_integer, int);
big_integer operator<<(big_integer, int);
#endif
