#ifndef LU_SOURCE_H
#define LU_SOURCE_H

#include <istream>

#include "string.h"
#include "internal/constexpr.h"

namespace lu
{
// source_reader is more precise for intended purpose.
// the source must outlive all compilation processes
struct source
{
    const static size_t npos = string_view::npos;

    source(string&& name);
    source(string&& name, string&&, encoding);
    source(source&&);

    // TODO encoding
    static source from_stream(string&& name, std::istream& stream);
    static source from_string(string&& name, string&& str);

    size_t size() const;

    string_view name() const { return _name; }
    string::CharT operator[](size_t) const;
    string_view read(size_t pos, size_t len = npos) const;
    const encoding& enc() const { return _enc; }
private:
    string _name; // id (file path or other name)
    string _text; // TODO read in chunks (not all at once)
    encoding _enc;
};

}

#endif // LU_SOURCE_H
