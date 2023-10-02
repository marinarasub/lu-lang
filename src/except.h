#ifndef LU_EXCEPT_H
#define LU_EXCEPT_H

#include <stdexcept>
#include "string.h"
#include "internal/debug.h"

namespace lu
{
struct lu_except : public std::runtime_error
{
public:
    virtual const char* what() const noexcept override
    {
        return _msg.buffer();
    }

protected:
    lu_except(string_view msg) : std::runtime_error(""), _msg(msg) {}

private:
    string _msg;

};

struct internal_except : public lu_except
{
    internal_except(string_view msg) : lu_except(string::join("internal error '", msg, "', contact <PLACEHOLDER> for help")) {}
};

#define internal_except_with_location(msg) internal_except(string::join(__FILE__, ":", to_string(__LINE__), ": ", LU_FUNCTION_NAME, ": ", msg))
#define internal_except_todo() internal_except_with_location("TODO")
#define internal_except_unhandled_switch(msg) internal_except_with_location(string::join("illegal or unhandled switch case: ", msg))

}

#endif // LU_EXCEPT_H
