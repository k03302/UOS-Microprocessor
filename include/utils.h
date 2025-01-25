#ifndef UTILS_H
#define UTILS_H

#include <assert.h>

inline int clamp(int value, int min, int max)
{
    if (value > max)
    {
        return max;
    }
    if (value < min)
    {
        return min;
    }
    return value;
}

#endif