#include "stdafx.h"
#include "Utils.h"

int ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec)
{
    int l_result = -1;
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
