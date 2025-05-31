#pragma once
#include <algorithm>
#include <cstring>
#include <ostream>

class string
{
    using size_t = unsigned long long;

    size_t m_cap = 0;
    size_t m_sz = 0;
    char* m_arr = nullptr;

public:
    string() = default;
    string(size_t sz, char c)
        : string(sz)
    {
        std::fill(m_arr, m_arr + m_sz, c);
    }
    explicit string(const char* str)
        : string(std::strlen(str))
    {
        std::copy(str, str + m_sz, m_arr);
    }

    ~string()
    {
        // safe to delete nullptr pointer
        delete[] m_arr;
    }

    string(const string& other)
        : string(other.m_sz)
    {
        // memcpy - for simple types
        // memmove - works with intersect diapasons
        std::copy(other.m_arr, other.m_arr + m_sz, m_arr);
    }

    /*string& operator=(const string& other)
    {
        if (this != &other) {
            delete[] m_arr;

            m_cap = other.m_cap;
            m_sz = other.m_sz;
            m_arr = new char[m_cap];
            std::copy(other.m_arr, other.m_arr + m_sz + 1, m_arr);
        }
        return *this;
    }*/

    // Copy and swap idiom. Assigment to yourself is bad
    // ref qualifier & is only for lvalue assigments
    string& operator=(string other) &
    {
        swap(other);
        return *this;
    }


    // if we have const or ref in fields, compiler won't generate default ctor assigment

    void swap(string& other)
    {
        std::swap(m_cap, other.m_cap);
        std::swap(m_sz, other.m_sz);
        std::swap(m_arr, other.m_arr);
    }

    char& operator[](size_t idx)
    {
        return m_arr[idx];
    }

    // Why not char as return value
    // string s = "abcd";
    // const string& cs = s;
    // const char& c = cs.front();
    // s[0] = 'b';
    // assert(c == 'b');
    const char& operator[](size_t idx) const
    {
        return m_arr[idx];
    }

    string& operator+=(const string& other)
    {
        this->append(other);
        return *this;
    }

    // C++20 Three-way comparison (spaceship)
    //operator <=>(const string& other) = default;

    const char* c_str() const
    {
        return m_arr;
    }

    size_t size() const
    {
        return m_sz;
    }

    std::ostream& operator<<(std::ostream& os) const
    {
        return os << m_arr;
    }

public:
    void append(const string& other)
    {
        // TODO: make more effective if use copaсity
        if (other.m_arr == nullptr)
            return;

        string result = string(m_arr, m_sz, other.m_arr, other.m_sz);
        swap(result);
    }

private:
    // Delegating constructor
    // Creates a base string
    // Initializing fields in initializing lists. In constructor's body fields have already initialized.
    string(size_t sz)
        : m_cap(sz + 1)
        , m_sz(sz)
        , m_arr(new char[m_cap])
    {
        m_arr[sz] = '\0';
    }

    string(const char* str1, size_t sz1, const char* str2, size_t sz2)
        : string(sz1 + sz2)
    {
        std::copy(str1, str1 + sz1, m_arr);
        std::copy(str2, str2 + sz2, m_arr + sz1);
    }
};

// const as a return to forbid assigment to rvalue (until C++11. since - use ref qualifiers):
// a + b = c; // where a, b, c are string
// RVO
string operator+(const string& s1, const string& s2)
{
    string result = s1;
    result += s2;
    return result;
}

// RVO won't work - unnecessary copy
/*
 *  string operator+(string s1, const string& s2)
 *  {
 *      return s1 += s2;
 *  }
*/


bool operator<(const string& lhs, const string& rhs)
{
    return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
}
// interesting impl
bool operator>(const string& lhs, const string& rhs)
{
    return rhs < lhs;
}

bool operator==(const string& lhs, const string& rhs)
{
    return std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
}
bool operator!=(const string& lhs, const string& rhs)
{
    return !(lhs == rhs);
}

// TODO: operator>>. string non const
std::ostream& operator<<(std::ostream& os, const string& s)
{
    return s.operator<<(os);
}

string operator""_s(const char * str, size_t)
{
    return string(str);
}

// string s = s - exec copy constructor

// contextual conversion
