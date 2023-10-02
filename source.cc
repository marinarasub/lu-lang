#include "source.h"

#include "utility.h"

#include <cassert>

namespace lu
{

source::source(string&& name) : _name(move(name)), _text(""), _enc() {}

source::source(string&& name, string&& text, encoding enc) : _name(move(name)), _text(move(text)), _enc(enc) {}

source::source(source&& other) : _name(move(other._name)), _text(move(other._text)), _enc(move(other._enc)) {}

// TODO encoding
source source::from_stream(string&& name, std::istream& stream)
{
    string str;
    char buf[4096];
    if (!stream.good())
    {
        throw "TODO PROPER ERROR";
    }
    while (stream.read(buf, sizeof(buf)))
    {
        str.append(buf, sizeof(buf));
    }
    if (!stream.eof())
    {
        throw "TODO THROW PROPER";
    }
    str.append(buf, static_cast<size_t>(stream.gcount()));

    return source(move(name), move(str), encoding());
}

source source::from_string(string&& name, string&& str)
{
    source src(move(name), move(str), encoding());
    return src;
}

size_t source::size() const
{
    return _text.size();
}

string::CharT source::operator[](size_t pos) const
{
    return _text[pos];
}

string_view source::read(size_t pos, size_t len) const
{
    return string_view(_text).subview(pos, len);
}

}
