#ifndef __MATHS_H__
#define __MATHS_H__

#include <algorithm>
#include <cmath>

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
double map_double(double value, double istart, double istop, double ostart, double ostop);

double radians_to_degrees(double radians);

double get_distance(double x1, double y1, double x2, double y2);

}; // end of namespace

#endif

