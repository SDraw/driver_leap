#include "stdafx.h"
#include "Utils.h"

size_t ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = f_vec.begin(), iterEnd = f_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(f_val))
        {
            l_result = std::distance(f_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}

size_t ReadEnumVector(const char *f_val, const std::vector<std::string> &f_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = f_vec.begin(), iterEnd = f_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(f_val))
        {
            l_result = std::distance(f_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}
