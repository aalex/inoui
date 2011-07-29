#ifndef __MATHS_H__
#define __MATHS_H__

#include <algorithm>
#include <cmath>
#include "maths.h"

namespace inoui
{

double map_double(double value, double istart, double istop, double ostart, double ostop)
{
    double ret = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    // In Processing, they don't do the following: (clipping)
    return std::max(std::min(ret, ostop), ostart);
}


double radians_to_degrees(double radians)
{
    static double ratio = 180.0 / 3.141592653589793238;
    return radians * ratio;
}

double get_distance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(
            std::abs(std::pow(std::abs(x2 - x1), 2.0)) + 
            std::abs(std::pow(std::abs(y2 - y1), 2.0))
        );
}

}; // end of namespace

#endif

