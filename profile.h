#ifndef LU_PROFILE_H
#define LU_PROFILE_H

#include <functional>
#include <iostream>
#include <cstddef>
#include "adt/vector.h"
#include "string.h"

namespace lu
{
namespace profile
{

struct time_settings
{
    vector<size_t> sizes;
    string_view name; // name of test
};

void time(const std::function<void()>& cb, const time_settings&, std::ostream& result_os);

}
}

#endif // LU_PROFILE_H
