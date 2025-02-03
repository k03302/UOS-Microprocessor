#ifndef MATH_H
#define MATH_H

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

inline int min(int a, int b)
{
    return a < b ? a : b;
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

#endif