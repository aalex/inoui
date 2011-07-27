#include "map.h"
#include <cmath>
#include <clutter/clutter.h>

Map::Map() :
    scale_(1.0)
{
    closest_point_ = 0;
    group_ = clutter_group_new();
    clutter_actor_set_name(group_, "map-group");
}

ClutterActor *Map::get_actor()
{
    return group_;
}

void Map::set_scale(double scale)
{
    scale_ = scale;
}

bool Map::set_selected(Point *selected)
{
    if (closest_point_ == selected)
        return false;
    // else..
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

Point *Map::add_point(double x, double y)
{
    points_.push_back(PointPtr(new Point(scale_, x, y)));
    Point *point = points_.at(points_.size() - 1).get();
    clutter_container_add_actor(CLUTTER_CONTAINER(group_), point->get_actor());
    return point;
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
            g_print("Point (%f, %f) is %f units far from (%f, %f).\n", point->get_x(), point->get_y(), distance, x, y);
            if (distance < smallest_distance)
            {
                smallest_distance = distance;
                closest = index;
            }
            ++index;
        }
        Point *ret = points_.at(closest).get();
        if (ret != closest_point_)
        {
            closest_point_ = ret;
            point_chosen_signal_(ret->get_next_sound());
        }
        return ret;
    }
}

