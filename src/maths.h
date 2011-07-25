#ifndef __MATHS_H__
#define __MATHS_H__

#include <algorithm>

namespace inoui
{

/**
 * Convenience function to map a variable from one coordinate space
 * to another.
 * The result is clipped in the range [ostart, ostop]
 * Make sure ostop is bigger than ostart.
 *
 * To map a MIDI control value into the [0,1] range:
 * map(value, 0.0, 1.0, 0. 127.);
 *
 * Depends on: #include <algorithm>
 */
double map_double(double value, double istart, double istop, double ostart, double ostop)
{
    double ret = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    // In Processing, they don't do the following: (clipping)
    return std::max(std::min(ret, ostop), ostart);
}

double mult(double value, double factor)
{
    return value * factor;
}

double demult(double value, double factor)
{
    return value / factor;
}

double radians_to_degrees(double radians)
{
    static double ratio = 180.0 / 3.141592653589793238;
    return radians * ratio;
}

};
#endif

