#include "map.h"
#include <cmath>

bool Map::set_selected(Point *selected)
{
    bool got_it = false;
    PointIterator iter;
    for (iter = points_.begin(); iter < points_.end(); ++iter)
    {
        Point *current = (*iter).get();
        if (selected)
        {
            if (current == selected)
                current->set_selected(true);
            else
                current->set_selected(false);
        }
        else
            current->set_selected(false);
    }
    if (selected)
        return got_it;
    else
        return true;
}

Point *Map::add_point(ClutterContainer *parent, double x, double y)
{
    points_.push_back(PointPtr(new Point(parent, x, y)));
    return points_.at(points_.size() - 1).get();
}

static double get_distance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(
            std::abs(std::pow(std::abs(x2 - x1), 2.0)) + 
            std::abs(std::pow(std::abs(y2 - y1), 2.0))
        );
}

Point *Map::get_closest_point(double x, double y)
{
    if (points_.size() == 0)
        return 0;
    else
    {
        unsigned int closest = 0;
        unsigned int index = 0;
        double smallest_distance = 999999999999.0;
        //g_print("Comparing to point (%f, %f)\n", x, y);
        PointIterator iter;
        for (iter = points_.begin(); iter < points_.end(); ++iter)
        {
            Point *point = (*iter).get();
            double distance = get_distance(x, y, point->get_x(), point->get_y());
            //g_print("Point (%f, %f) is %f units far.\n", point->get_x(), point->get_y(), distance);
            if (distance < smallest_distance)
            {
                smallest_distance = distance;
                closest = index;
            }
            ++index;
        }
        return points_.at(closest).get();
    }
}

