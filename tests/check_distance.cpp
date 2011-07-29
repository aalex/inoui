#include <iostream>
#include "maths.h"

static const double THRESHOLD = 0.1;

bool test(double x1, double y1, double x2, double y2, double expected)
{
    double result = inoui::get_distance(x1, y1, x2, y2);
    bool ok = ((expected - THRESHOLD) < result && result < (expected + THRESHOLD));
    if (! ok)
    {
        std::cout << "ERROR: distance between (" << x1 << "," << y1 << ") and (" << x2 << "," << y2 << ") is " << result << " but should be " << expected << "." << std::endl;
        return false;
    }
    return ok;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (! test(0.0, 0.0, 1.0, 1.0, 1.414213562))
        return 1;
    return 0;
}

