#include "print.h"
#include "string.h"
#include "byte.h"

#include <cctype>

namespace lu
{
template <typename I, typename = typename std::enable_if<std::is_integral<I>::value>::type>
inline string hexstr_lower(I val, size_t width = sizeof(I) * 2)
{
    static const char digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', };
    string str(width, '0');
    for (size_t i = 0, j = (width - 1) * 4; i < width; ++i, j -= 4)
    {
        str[i] = digits[(val >> j) & 0x0f];
    }
    return str;
}

inline string hexstr_lower(const void* ptr, size_t width = sizeof(void*) * 2)
{
    uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
    static const char digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', };
    string str(width, '0');
    for (size_t i = 0, j = (width - 1) * 4; i < width; ++i, j -= 4)
    {
        str[i] = digits[(val >> j) & 0x0f];
    }
    return str;
}

template <typename T>
inline string hexstr(const T& val) // TODO leading 0, lower/upper, 0x 
{
    return hexstr_lower(val);
}

string hex(int i) { return hexstr(i); }
string hex(long l) { return hexstr(l); }
string hex(long long ll) { return hexstr(ll); }
string hex(unsigned u) { return hexstr(u); }
string hex(unsigned long ul) { return hexstr(ul); }
string hex(unsigned long long ull) { return hexstr(ull); }

// TODO configurable hexdump (width, show char etc)
string hexdump(const unsigned char* data, size_t nbyte)
{
    string str;
    //str += "(";
    for (size_t i = 0; i < nbyte; i++)
    {
        if (!(i % 8))
        {
            if (i) str.append('\n');
            str.append(hexstr(&data[i])).append("  "); // address
        }
        else if (!(i % 4)) str.append(' ');
        str.append(hexstr(data[i])).append(' ');
    }
    str.append("\n");
    //str += "\n)";
    return str;
}

string escape(char c)
{
    if (std::isprint(static_cast<unsigned char>(c)))
    {
        return string(1, c);
    }
    else
    {
        switch (c)
        {
        case '\\':
            return "\\\\";
        case '\0':
            return "\\0";
        case '\n':
            return "\\n";
        case '\r':
            return "\\r";
        case '\t':
            return "\\t";
        case '\v':
            return "\\v";
        case '\f':
            return "\\f";
        case '\a':
            return "\\a";
        case '\b':
            return "\\b";
        default:
            return to_string("\\x").append(hexstr(c));
        }
    }
}

char unescape(string_view str, size_t& i)
{
    char c = str[i];
    if (c == '\\')
    {
        if (++i >= str.size())
        {
            throw 0; // TODO invalid escape - no next char
        }
        c = str[i++];
        switch (c)
        {
        case '\\':
            return '\\';
        case '0':
            return '\0';
        case 'r':
            return '\r';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case '"':
            return '"';
        default: // TODO others i guess, hex etc.
            return c;
        }
    }
    else
    {
        ++i;
        return c;
    }
    
}

string unescape(string_view str)
{
    string s; 
    s.reserve(str.size());
    size_t i = 0;
    while (i < str.size())
    {
        s.append(unescape(str, i));
    }
    return s;
}

string escape(string_view s)
{
    string str;
    str.reserve(s.size());
    for (const char& c : s)
    {
        str.append(escape(c));
    }
    return str;
}

string pad(string_view sv, size_t lpad, size_t rpad, string::CharT c)
{
    return string::join(string(lpad, c), sv, string(rpad, c));
}

void putascii(char c, std::ostream& os)
{
    os << c;
}

void print(string_view msg, std::ostream& os)
{
    os << msg;
}

void println(string_view msg, std::ostream& os)
{
    os << msg << '\n';
}

} // namespace lu
