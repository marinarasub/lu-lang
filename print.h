#ifndef LU_PRINT_H
#define LU_PRINT_H

#include "string.h"
#include "utility.h"
#include "byte.h"

#include <iostream>
#include <locale>

namespace lu
{
string hex(int);
string hex(long);
string hex(long long);
string hex(unsigned);
string hex(unsigned long);
string hex(unsigned long long);
    // TODO use byte isntead after byte is impl.
string hexdump(const unsigned char* data, size_t nbyte);

// TODO escape/unescape may need encoding.
string escape(char c);

// update i to index or next char to unescape (since escaping may advance multiple times)
char unescape(string_view, size_t& i);

string unescape(string_view);

string escape(string_view);

string pad(string_view, size_t lpad, size_t rpad, string::CharT c = ' ');

void putascii(char c, std::ostream& = std::cout);
void print(string_view, std::ostream& = std::cout);
void println(string_view, std::ostream& = std::cout);

//using locale = std::locale;
//using cout = std::basic_ostream<ascii>;
//using ucout = std::basic_ostream<unicode>;
// using std::cout;
// using std::wcout;
// using std::cerr;
// using std::wcerr;
// using std::clog;
// using std::wclog;
}

#endif // LU_PRINT_H
