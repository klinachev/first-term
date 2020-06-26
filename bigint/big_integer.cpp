#include <cstring>
#include "big_integer.h"

static const uint64_t UINT32MOD = 1ull << 32u;

static uint32_t* allocate_array(size_t size) {
    return static_cast<uint32_t *>(operator new(size * sizeof(uint32_t)));
}

uint32_t* big_integer::data() const {
    return data_;
}

void big_integer::swap(big_integer &y) {
    std::swap(size_, y.size_);
    std::swap(data_, y.data_);
}

bool big_integer::sign() const {
    return data()[size_ - 1] & (1u << 31u);
}

void big_integer::change_data(size_t new_size, uint32_t* data_pointer) {
    size_t sz = new_size - 1;
    if (new_size != 1 && (data_pointer[sz] == 0 || data_pointer[sz] == UINT32_MAX)) {
        uint32_t head = data_pointer[sz];
        for (; sz > 0 && data_pointer[sz - 1] == head; sz--);
        if (sz && ((data_pointer[sz] & (1u << 31u)) == (data_pointer[sz - 1] & (1u << 31u)))) sz--;
    }
    uint32_t* dop = data_pointer;
    if (sz + 1 < new_size) {
        try {
            dop = allocate_array(new_size);
        } catch (...) {
            operator delete(data_pointer);
            throw;
        }
        new_size = sz + 1;
        memcpy(dop, data_pointer, new_size * sizeof(uint32_t));
        operator delete (data_pointer);
    }
    data_pointer = data_;
    data_ = dop;
    operator delete(data_pointer);
    size_ = new_size;
}

big_integer::big_integer() : data_(allocate_array(1)), size_(1) {
    data_[0] = 0;
}

big_integer::big_integer(int a) : data_(allocate_array(1)), size_(1) {
    data_[0] = *reinterpret_cast<uint32_t*>(&a);
}

big_integer::big_integer(uint32_t a) {
    if (a > INT32_MAX) {
        data_ = allocate_array(2);
        size_ = 2;
        data_[0] = a;
        data_[1] = 0;
    } else {
        data_ = allocate_array(1);
        size_ = 1;
        data_[0] = a;
    }
}

big_integer::~big_integer() {
    operator delete(data_);
}

big_integer::big_integer(big_integer const &other) : data_(allocate_array(other.size_)), size_(other.size_) {
    memcpy(data_, other.data_, size_ * sizeof(uint32_t));
}

big_integer::big_integer(std::string const &str) : data_(allocate_array(1)), size_(1) {
    big_integer mul(1);
    data_[0] = 0;
    int len = str.length(), start = 0;
    bool sign_ = false;
    if (str[0] == '-') {
        sign_ = true;
        start = 1;
    }
    while (len - 9 >= start) {
        len -= 9;
        *this += (mul * big_integer(std::stoi(str.substr(len, 9))));
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
    if (a.size_ == b.size_) {
        for (size_t i = a.size_; i > 0; --i) {
            if (a.data()[i - 1] != b.data()[i - 1]) {
                return a.data()[i - 1] > b.data()[i - 1];
            }
        }
    }
    return a.size_ > b.size_;
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
    if (a.size_== b.size_) {
        for (size_t i = 0; i < a.size_; i++) {
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
    uint32_t toAdd = 0, i = 0, len = std::max(rhs.size_, size_);
    if ((size_< rhs.size_ && this_sign) || (size_ > rhs.size_ && rhs_sign))
        toAdd = UINT32_MAX;
    uint32_t *biggerData = this->data();
    if (size_< rhs.size_)
        biggerData = rhs.data();
    uint32_t *mas = allocate_array(len + 1);
    uint64_t rc = 0;
    for (; i < std::min(rhs.size_, size_); i++) {
        rc += static_cast<uint64_t>(this->data()[i]) + rhs.data()[i];
        mas[i] = rc % UINT32MOD;
        rc = rc >= UINT32MOD;
    }
    for (; i < len; i++) {
        rc += static_cast<uint64_t>(biggerData[i]) + toAdd;
        mas[i] = rc % UINT32MOD;
        rc = rc >= UINT32MOD;
    }
    mas[len] = rc != 0 ? UINT32_MAX : 0;
    if (this_sign != rhs_sign) {
        mas[len] = ((1u << 31u) & mas[len - 1]) ? UINT32_MAX : 0;
    }
    change_data(len + 1, mas);
    return *this;
}

big_integer &big_integer::operator-=(big_integer const& rhs) {
    *this += -rhs;
    return *this;
}

big_integer &big_integer::operator*=(big_integer b) {
    bool this_sign = sign(), rhs_sign = b.sign();
    big_integer a;
    if (this_sign) a = -*this;
    else a = *this;
    if (rhs_sign) b = -b;
    size_t res_size = a.size_ + b.size_ + 1;
    uint32_t *res_data = allocate_array(res_size);
    for (size_t i = 0; i < res_size; i++) {
        res_data[i] = 0;
    }
    for (size_t i = 0; i < size_; i++) {
        uint64_t dop = 0;
        uint32_t rc = 0;
        for (size_t j = 0; j < b.size_; j++) {
            dop = static_cast<uint64_t>(a.data()[i]) * b.data()[j] + res_data[i + j] + rc;
            res_data[i + j] = dop % UINT32MOD;
            rc = dop >> 32u;
        }
        res_data[b.size_ + i] = rc;
    }
    big_integer res;
    res.change_data(res_size, res_data);
    if (this_sign ^ rhs_sign) {
        *this = -res;
    } else {
        swap(res);
    }
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    big_integer r;
    uint32_t *div_data;
    size_t new_size = divide(*this, rhs, div_data, r);
    big_integer q;
    q.change_data(new_size, div_data);
    if (this->sign() ^ rhs.sign()) q = -q;
    swap(q);
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    big_integer r;
    uint32_t *div_data;
    divide(*this, rhs, div_data, r);
    operator delete (div_data);
    if (this->sign()) r = -r;
    swap(r);
    return *this;
}

big_integer big_integer::operator~() const {
    uint32_t *new_data = allocate_array(size_);
    for (size_t i = 0; i < size_; i++) {
        new_data[i] = ~this->data()[i];
    }
    big_integer cop;
    cop.change_data(size_, new_data);
    return cop;
}

big_integer& big_integer::apply_operation(big_integer const& other,
                                          std::function<uint32_t(uint32_t, uint32_t)> const& func) {
    size_t i = 0, new_size = std::max(size_, other.size_);
    uint32_t* new_data = allocate_array(new_size);
    for (; i < std::min(size_, other.size_); ++i) {
        new_data[i] = func(this->data()[i], other.data()[i]);
    }
    uint32_t toAdd = !other.sign() ? 0 : UINT32_MAX;
    for (; i < size_; i++) {
        new_data[i] = func(this->data()[i], toAdd);
    }
    toAdd = !sign() ? 0 : UINT32_MAX;
    for (; i < other.size_; i++) {
        new_data[i] = func(other.data()[i], toAdd);
    }
    change_data(new_size, new_data);
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
    uint32_t bigShift = rhs / 32u, shift = rhs % 32u;
    uint64_t rc = 0, dop = 0;
    uint32_t sizeM = size_+ bigShift + ((static_cast<uint64_t>(data()[size_ - 1]) << shift) >= (1u << 31u));
    uint32_t *mas = allocate_array(sizeM);
    for (size_t i = 0; i < bigShift; i++) mas[i] = 0;
    for (size_t i = 0; i < size_; i++) {
        dop = static_cast<uint64_t>(data()[i]) << shift;
        mas[i + bigShift] = (dop % UINT32MOD) + rc;
        rc = dop >> 32u;
    }
    if (sizeM > size_+ bigShift) mas[sizeM - 1] = rc;
    if (sign()) {
        uint64_t i = 1;
        while (i <= mas[sizeM - 1] && i <= (1u << 31u)) i <<= 1u;
        while (i <= (1u << 31u)) {
            mas[sizeM - 1] ^= i;
            i <<= 1u;
        }
    }
    change_data(sizeM, mas);
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (rhs < 0) {
        return *this <<= -rhs;
    }
    uint32_t bigShift = rhs / 32u, shift = rhs % 32u;
    if (bigShift > size_) {
        size_ = 1;
        data_[0] = 0;
        return *this;
    }
    uint32_t sizeM = size_- bigShift, next = 0;
    uint64_t dp = 0;
    auto *mas = allocate_array(sizeM);
    for (int i = (int) size_- 1; i >= (int)bigShift; i--) {
        dp = (static_cast<uint64_t>(data()[i])) << (32u - shift);
        mas[i - bigShift] = (dp >> 32u) + next;
        next = dp % UINT32MOD;
    }
    if (sign()) {
        uint64_t i = 1;
        while (i <= mas[sizeM - 1] && i <= (1u << 31u)) i <<= 1u;
        while (i <= (1u << 31u)) {
            mas[sizeM - 1] ^= i;
            i <<= 1u;
        }
    }
    change_data(sizeM, mas);
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
    uint32_t *mas = allocate_array(val.size_* 32 / 29 + 1);
    int len = 0;
    big_integer q, r;
    try {
        while (val.size_ > 1 || val.data()[0] != 0) {
            mas[len++] = val.small_div(1000000000);
        }
        st.append(std::to_string(mas[len - 1]));
        for (int i = len - 2; i >= 0; i--) {
            cop = std::to_string(mas[i]);
            for (size_t j = 0; j < 9 - cop.length(); j++) {
                st.append("0");
            }
            st.append(cop);
        }
    } catch (...) {
        operator delete(mas);
        throw;
    }
    operator delete(mas);
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

uint32_t big_integer::trial(big_integer const&r, big_integer const&d, uint32_t m, uint32_t k) {
    uint32_t km = m + k - 1;
    uint64_t rr = (static_cast<uint64_t> (r.data()[km]) << 32u) + r.data()[km - 1];
    uint64_t dd = d.data()[m - 1];
    return std::min(rr / dd, static_cast<uint64_t >(UINT32_MAX));
}

bool big_integer::smaller(big_integer const &dq, uint32_t k, uint32_t m) const {
    uint32_t i = m;
    for (; 0 < i; i--) {
        if (data()[i + k - 1] != dq.data()[i - 1]) {
            return data()[i + k - 1] < dq.data()[i - 1];
        }
    }
    return false;
}

void big_integer::difference(big_integer const&dq, uint32_t k, uint32_t m) {
    int64_t borrow = 0;
    for (uint32_t i = 0; i < m; i++) {
        uint64_t diff = static_cast<uint64_t> (data()[i + k - 1]) + UINT32MOD - dq.data()[i] - borrow;
        data()[i + k - 1] = diff % UINT32MOD;
        borrow = 1 - diff / UINT32MOD;
    }
}

size_t big_integer::long_divide(big_integer& x, big_integer& y, uint32_t* &div_data, big_integer& r) {
    uint32_t n, m = y.size_, xs;
    for (; m > 0 && y.data()[m - 1] == 0; m--);
    uint32_t f = UINT32MOD / (1ull + y.data()[m - 1]);
    y *= big_integer(f);
    m = y.size_;
    for (; m > 0 && y.data()[m - 1] == 0; m--);
    r = x * big_integer(f);
    n = r.size_;
    size_t div_size = n - m + 1;
    div_data = allocate_array(div_size);
    div_data[div_size - 1] = 0;
    for (uint32_t k = n - m; k > 0; --k) {
        if (r.data()[k + m - 1] || r.data()[k + m - 2]) {
            uint32_t qt = trial(r, y, m, k);
            if (qt == 0) {
                div_data[k - 1] = 0;
            } else {
                x = y * big_integer(qt);
                xs = m + 1;
                while (r.smaller(x, k - 1, xs)) {
                    qt--;
                    if (qt == 0) {
                        div_data[k - 1] = 0;
                        break;
                    }
                    x = y * qt;
                    xs = m + 1;
                }
                if (qt == 0) continue;
                div_data[k - 1] = qt;
                r.difference(x, k, xs);
            }
        } else {
            div_data[k - 1] = 0;
        }
    }
    r.small_div(f);
    return div_size;
}

size_t big_integer::divide(big_integer x, big_integer y, uint32_t* &div_data, big_integer &r) {
    if (x.sign()) x = -x;
    if (y.sign()) y = -y;
    if (y == 0) {
        throw std::overflow_error("Divide by zero exception");
    }
    if (x == 0) {
        div_data = allocate_array(1);
        div_data[0] = 0;
        return 1;
    }
    if (y.size_> x.size_) {
        div_data = allocate_array(1);
        div_data[0] = 0;
        r = x;
        return 1;
    }
    return long_divide(x, y, div_data, r);
}

uint32_t big_integer::small_div(uint32_t divisor) {
    uint32_t *cop_data = allocate_array(size_);
    uint32_t mod = 0;
    for (size_t i = size_; i > 0; i--) {
        uint64_t rc = (static_cast<uint64_t >(mod) << 32u) + data()[i - 1];
        cop_data[i - 1] = rc / divisor;
        mod = rc % divisor;
    }
    change_data(size_, cop_data);
    return mod;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer const big_integer::operator++(int) {
    big_integer copy(*this);
    *this += 1;
    return copy;
}

big_integer &big_integer::operator--() {
    return *this += -1;
}

big_integer const big_integer::operator--(int) {
    big_integer copy(*this);
    *this += -1;
    return copy;
}


