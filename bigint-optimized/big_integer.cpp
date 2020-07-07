#include <cstring>
#include <iostream>
#include <assert.h>
#include "big_integer.h"

static const uint64_t UINT32MOD = 1ull << 32u;

namespace {
    my_buffer::my_buffer() {
        is_static = true;
        static_buf.size_ = 1;
        static_buf.data_[0] = 0;
    }

    my_buffer::my_buffer(size_t size) {
        if (size > MAX_STATIC_SIZE) {
            is_static = false;
            dynamic_buf = static_cast<dynamic_buffer *>(operator new (sizeof(dynamic_buffer) + size * sizeof(uint32_t)));
            dynamic_buf->ref_counter = 1;
            dynamic_buf->size_ = size;
        } else {
            is_static = true;
            static_buf.size_ = size;
        }
    }

    my_buffer::my_buffer(my_buffer const &other) {
        is_static = other.is_static;
        if (is_static) {
            memcpy(&static_buf, &other.static_buf, sizeof(static_buffer));
        } else {
            dynamic_buf = other.dynamic_buf;
            dynamic_buf->ref_counter++;
        }
    }

    my_buffer::~my_buffer() {
        if (!is_static) {
            --dynamic_buf->ref_counter;
            if (dynamic_buf->ref_counter == 0) {
                operator delete(dynamic_buf);
            }
        }
    }

    size_t my_buffer::size() const {
        return is_static ? static_buf.size_ : dynamic_buf->size_;
    }

    uint32_t const *my_buffer::data() const {
        return is_static ? static_buf.data_ : dynamic_buf->data_;
    }

    uint32_t *my_buffer::non_const_data() {
        return is_static ? static_buf.data_ : dynamic_buf->data_;
    }

    void my_buffer::swap(my_buffer &other) {
        std::swap(is_static, other.is_static);
        if (sizeof(static_buf) > sizeof(dynamic_buf)) {
            std::swap(this->static_buf, other.static_buf);
        } else {
            std::swap(this->dynamic_buf, other.dynamic_buf);
        }
    }

    void my_buffer::change_capacity(size_t new_size) {
        my_buffer copy(new_size);
        this->swap(copy);
    }
}

uint32_t const *big_integer::data() const {
    return buf.data();
}

uint32_t *big_integer::non_const_data() {
    return buf.non_const_data();
}

void big_integer::swap(big_integer &other) {
    buf.swap(other.buf);
}

bool big_integer::sign() const {
    return data()[buf.size() - 1] & (1u << 31u);
}

void big_integer::change_data(std::vector<uint32_t> &new_buf) {
    size_t sz = new_buf.size() - 1;
    if (new_buf.size() != 1 && (new_buf[sz] == 0 || new_buf[sz] == UINT32_MAX)) {
        uint32_t head = new_buf[sz];
        for (; sz > 0 && new_buf[sz - 1] == head; sz--);
        if (sz && ((new_buf[sz] & (1u << 31u)) == (new_buf[sz - 1] & (1u << 31u)))) {
            sz--;
        }
    }
    my_buffer my_new_buff(sz + 1);
    if (sz + 1 <= MAX_STATIC_SIZE) {
        memcpy(my_new_buff.static_buf.data_, new_buf.data(), (sz + 1) * sizeof(uint32_t));
    } else {
        memcpy(my_new_buff.dynamic_buf->data_, new_buf.data(), (sz + 1) * sizeof(uint32_t));
    }
    buf.swap(my_new_buff);
}

void big_integer::clear_empty_slots() {
    std::vector<uint32_t> new_data(buf.size());
    for (size_t i = 0; i < buf.size(); ++i) {
        new_data[i] = buf.data()[i];
    }
    change_data(new_data);
}

big_integer::big_integer() : buf() {}

big_integer::big_integer(int32_t a) {
    buf.is_static = true;
    buf.static_buf.size_ = 1;
    buf.static_buf.data_[0] = *reinterpret_cast<uint32_t *>(&a);
}

big_integer::big_integer(uint32_t a) {
    buf.is_static = true;
    if (a > INT32_MAX) {
        buf.static_buf.size_ = 2;
        buf.static_buf.data_[0] = a;
        buf.static_buf.data_[1] = 0;
    } else {
        buf.static_buf.size_ = 1;
        buf.static_buf.data_[0] = a;
    }
}

big_integer::~big_integer() = default;

big_integer::big_integer(big_integer const &other) : buf(other.buf) {}

big_integer::big_integer(std::string const &str) : buf() {
    big_integer mul(1);
    size_t len = str.length(), start = 0;
    bool sign_ = false;
    if (str[0] == '-') {
        sign_ = true;
        start = 1;
    }
    while (len >= start + 9) {
        len -= 9;
        *this += mul * big_integer(std::stoi(str.substr(len, 9)));
        mul *= big_integer(1000000000);
    }
    if (len > start) {
        *this += mul * (std::stoi(str.substr(start, len - start)));
    }
    if (sign_) {
        *this = -*this;
    }
}

big_integer &big_integer::operator=(big_integer const &other) {
    if (this != &other) {
        big_integer copy(other);
        swap(copy);
    }
    return *this;
}

bool operator>(big_integer const &a, big_integer const &b) {
    if (a.sign() != b.sign()) {
        return b.sign();
    }
    if (a.buf.size() == b.buf.size()) {
        for (size_t i = a.buf.size(); i > 0; --i) {
            if (a.data()[i - 1] != b.data()[i - 1]) {
                return a.data()[i - 1] > b.data()[i - 1];
            }
        }
    }
    return a.buf.size() > b.buf.size();
}

bool operator<(big_integer const &a, big_integer const &b) {
    return b > a;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(b > a);
}

big_integer big_integer::operator-() const {
    return ~*this + 1;
}

big_integer big_integer::operator+() const {
    return *this;
}

bool operator==(big_integer const &a, big_integer const &b) {
    if (a.buf.size() == b.buf.size()) {
        for (size_t i = 0; i < a.buf.size(); i++) {
            if (a.data()[i] != b.data()[i]) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    bool this_sign = sign(), rhs_sign = rhs.sign();
    uint32_t to_add = 0;
    size_t i = 0;
    if ((buf.size() < rhs.buf.size() && this_sign) || (buf.size() > rhs.buf.size() && rhs_sign)) {
        to_add = UINT32_MAX;
    }
    uint32_t const *bigger_data = this->data();
    if (buf.size() < rhs.buf.size()) {
        bigger_data = rhs.data();
    }
    std::vector<uint32_t> vector(std::max(rhs.buf.size(), buf.size()) + 1);
    uint64_t rc = 0;
    for (; i < std::min(rhs.buf.size(), buf.size()); i++) {
        rc += static_cast<uint64_t>(this->data()[i]) + rhs.data()[i];
        vector[i] = rc % UINT32MOD;
        rc = rc >= UINT32MOD;
    }
    for (; i < vector.size() - 1; i++) {
        rc += static_cast<uint64_t>(bigger_data[i]) + to_add;
        vector[i] = rc % UINT32MOD;
        rc = rc >= UINT32MOD;
    }
    vector[vector.size() - 1] = rc != 0 ? UINT32_MAX : 0;
    if (this_sign != rhs_sign) {
        vector[vector.size() - 1] = ((1u << 31u) & vector[vector.size() - 2]) ? UINT32_MAX : 0;
    }
    change_data(vector);
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    *this += -rhs;
    return *this;
}

big_integer &big_integer::operator*=(big_integer const& other) {
    bool this_sign = sign(), rhs_sign = other.sign();
    size_t size_a = this->buf.size(), size_b = other.buf.size();
    const uint32_t *a = this->data(), *b = other.data();
    big_integer c, d;
    if (this_sign) {
        c = -*this;
        a = c.data();
        size_a = c.buf.size();
    }
    if (rhs_sign) {
        d = -other;
        b = d.data();
        size_b = d.buf.size();
    }
    std::vector<uint32_t> res_data(size_a + size_b + 1, 0);
    for (size_t i = 0; i < size_a; i++) {
        uint64_t dop = 0;
        uint32_t rc = 0;
        for (size_t j = 0; j < size_b; j++) {
            dop = static_cast<uint64_t>(a[i]) * b[j] + res_data[i + j] + rc;
            res_data[i + j] = dop % UINT32MOD;
            rc = dop >> 32u;
        }
        res_data[size_b + i] = rc;
    }
    big_integer res;
    res.change_data(res_data);
    if (this_sign ^ rhs_sign) {
        *this = -res;
    } else {
        swap(res);
    }
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    big_integer d, r;
    divide(*this, rhs, d, r);
    if (this->sign() ^ rhs.sign()) {
        d = -d;
    } else {
        d.clear_empty_slots();
    }
    swap(d);
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    big_integer d, r;
    divide(*this, rhs, d, r);
    if (this->sign()) {
        *this = -r;
    } else {
        swap(r);
    }
    return *this;
}

big_integer big_integer::operator~() const {
    std::vector<uint32_t> new_data(buf.size());
    for (size_t i = 0; i < buf.size(); i++) {
        new_data[i] = ~this->data()[i];
    }
    big_integer cop;
    cop.change_data(new_data);
    return cop;
}

big_integer &big_integer::apply_operation(big_integer const &other,
                                          std::function<uint32_t(uint32_t, uint32_t)> const &func) {
    std::vector<uint32_t> new_data(std::max(buf.size(), other.buf.size()));
    size_t i = 0;
    for (; i < std::min(buf.size(), other.buf.size()); ++i) {
        new_data[i] = func(this->data()[i], other.data()[i]);
    }
    uint32_t to_add = !other.sign() ? 0 : UINT32_MAX;
    for (; i < buf.size(); i++) {
        new_data[i] = func(this->data()[i], to_add);
    }
    to_add = !sign() ? 0 : UINT32_MAX;
    for (; i < other.buf.size(); i++) {
        new_data[i] = func(other.data()[i], to_add);
    }
    change_data(new_data);
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return apply_operation(rhs, [](uint32_t a, uint32_t b) { return a & b; });
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    return apply_operation(rhs, [](uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    return apply_operation(rhs, [](uint32_t a, uint32_t b) { return a | b; });
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (rhs < 0) {
        return *this >>= -rhs;
    }
    uint32_t big_shift = rhs / 32u, shift = rhs % 32u;
    uint64_t rc = 0, dop = 0;
    std::vector<uint32_t> mas(buf.size() + big_shift
                              + ((static_cast<uint64_t>(data()[buf.size() - 1]) << shift) >= (1u << 31u)));
    for (size_t i = 0; i < big_shift; i++) mas[i] = 0;
    for (size_t i = 0; i < buf.size(); i++) {
        dop = static_cast<uint64_t>(data()[i]) << shift;
        mas[i + big_shift] = (dop % UINT32MOD) + rc;
        rc = dop >> 32u;
    }
    if (mas.size() > buf.size() + big_shift) {
        mas[mas.size() - 1] = rc;
    }
    if (sign()) {
        uint64_t i = 1;
        while (i <= mas[mas.size() - 1] && i <= (1u << 31u)) i <<= 1u;
        while (i <= (1u << 31u)) {
            mas[mas.size() - 1] ^= i;
            i <<= 1u;
        }
    }
    change_data(mas);
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (rhs < 0) {
        return *this <<= -rhs;
    }
    uint32_t big_shift = rhs / 32u, shift = rhs % 32u;
    if (big_shift > buf.size()) {
        *this = 0;
        return *this;
    }
    uint32_t next = 0;
    uint64_t dp = 0;
    std::vector<uint32_t> mas(buf.size() - big_shift);
    for (size_t i = buf.size(); i > big_shift; i--) {
        dp = (static_cast<uint64_t>(data()[i - 1])) << (32u - shift);
        mas[i - 1 - big_shift] = (dp >> 32u) + next;
        next = dp % UINT32MOD;
    }
    if (sign()) {
        uint64_t i = 1;
        while (i <= mas[mas.size() - 1] && i <= (1u << 31u)) i <<= 1u;
        while (i <= (1u << 31u)) {
            mas[mas.size() - 1] ^= i;
            i <<= 1u;
        }
    }
    change_data(mas);
    return *this;
}

std::string to_string(big_integer val) {
    if (val == 0) {
        return "0";
    }
    std::string st, cop;
    if (val.sign()) {
        st = "-";
        val = -val;
    }
    std::vector<uint32_t> mas;
    big_integer q, r;
    while (val.buf.size() > 1 || val.data()[0] != 0) {
        mas.push_back(val.div_by_uint32_t(1000000000));
    }
    st.append(std::to_string(mas[mas.size() - 1]));
    for (size_t i = mas.size() - 1; i > 0; i--) {
        cop = std::to_string(mas[i - 1]);
        for (size_t j = 0; j < 9 - cop.length(); j++) {
            st.append("0");
        }
        st.append(cop);
    }
    return st;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

uint32_t big_integer::get_trial_multiplier(big_integer const &r, big_integer const &d, size_t m, size_t k) {
    uint32_t km = m + k - 1;
    uint64_t rr = (static_cast<uint64_t> (r.data()[km]) << 32u) + r.data()[km - 1];
    uint64_t dd = d.data()[m - 1];
    return std::min(rr / dd, static_cast<uint64_t>(UINT32_MAX));
}

bool big_integer::smaller(big_integer const &dq, size_t k, size_t m) const {
    uint32_t i = m;
    for (; 0 < i; i--) {
        if (data()[i + k - 1] != dq.data()[i - 1]) {
            return data()[i + k - 1] < dq.data()[i - 1];
        }
    }
    return false;
}

void big_integer::difference(big_integer const &dq, size_t k, size_t m) {
    int64_t borrow = 0;
    for (size_t i = 0; i < m; i++) {
        uint64_t diff = static_cast<uint64_t> (data()[i + k - 1]) + UINT32MOD - dq.data()[i] - borrow;
        non_const_data()[i + k - 1] = diff % UINT32MOD;
        borrow = 1 - diff / UINT32MOD;
    }
}

void big_integer::long_divide(big_integer &x, big_integer &y, big_integer &d, big_integer &r) {
    size_t n, m = y.buf.size(), xs;
    for (; m > 0 && y.data()[m - 1] == 0; m--);
    uint32_t f = UINT32MOD / (1ull + y.data()[m - 1]);
    y *= big_integer(f);
    m = y.buf.size();
    for (; m > 0 && y.data()[m - 1] == 0; m--);
    r = x * big_integer(f);
    n = r.buf.size();
    d.buf.change_capacity(n - m + 1);
    d.non_const_data()[d.buf.size() - 1] = 0;
    for (size_t k = n - m; k > 0; --k) {
        if (r.data()[k + m - 1] || r.data()[k + m - 2]) {
            uint32_t qt = get_trial_multiplier(r, y, m, k);
            if (qt == 0) {
                d.non_const_data()[k - 1] = 0;
            } else {
                x = y * big_integer(qt);
                xs = m + 1;
                while (r.smaller(x, k - 1, xs)) {
                    qt--;
                    if (qt == 0) {
                        d.non_const_data()[k - 1] = 0;
                        break;
                    }
                    x = y * qt;
                    xs = m + 1;
                }
                if (qt == 0) {
                    continue;
                }
                d.non_const_data()[k - 1] = qt;
                r.difference(x, k, xs);
            }
        } else {
            d.non_const_data()[k - 1] = 0;
        }
    }
    r.div_by_uint32_t(f);
}

void big_integer::divide(big_integer x, big_integer y, big_integer &d, big_integer &r) {
    if (x == 0) {
        d = 0;
        return;
    }
    if (y == 0) {
        throw std::overflow_error("Divide by zero exception");
    }
    if (x.sign()) {
        x = -x;
    }
    if (y.sign()) {
        y = -y;
    }
    if (y.buf.size() > x.buf.size()) {
        d = 0;
        r.swap(x);
        return;
    }
    if (y.buf.size() == 1 || (y.buf.size() == 2 && y.data()[1] == 0)) {
        d.swap(x);
        r = d.div_by_uint32_t(y.data()[0]);
    } else {
        long_divide(x, y, d, r);
    }
}

uint32_t big_integer::div_by_uint32_t(uint32_t divisor) {
    std::vector<uint32_t> cop_data(buf.size());
    uint32_t mod = 0;
    for (size_t i = buf.size(); i > 0; i--) {
        uint64_t rc = (static_cast<uint64_t>(mod) << 32u) + data()[i - 1];
        cop_data[i - 1] = rc / divisor;
        mod = rc % divisor;
    }
    change_data(cop_data);
    return mod;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer copy(*this);
    *this += 1;
    return copy;
}

big_integer &big_integer::operator--() {
    return *this += -1;
}

big_integer big_integer::operator--(int) {
    big_integer copy(*this);
    *this += -1;
    return copy;
}

void big_integer::printdata() {
    std::cout << buf.size() << " start \n";
    for (size_t i = 0; i < buf.size(); ++i) {
        std::cout << data()[i] << ' ';
    }
    std::cout << " end \n";
}


