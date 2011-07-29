#ifndef __MAP_H__
#define __MAP_H__

#include <clutter/clutter.h>
#include <vector>
#include <tr1/memory>
#include <boost/signals2.hpp>

#include "point.h"

class Map
{
    public:
        Map();
        Point *add_point(double x, double y);
        Point *get_closest_point(double x, double y);
        /**
         * Sets the selected point.
         */
        bool set_selected_point(Point *selected);
        boost::signals2::signal<void (std::string)> point_chosen_signal_;
        ClutterActor *get_actor();
        // typedefs:
        typedef std::tr1::shared_ptr<Point> PointPtr ;
        typedef std::vector<PointPtr>::iterator PointIterator ;
    private:
        std::vector<std::tr1::shared_ptr<Point> > points_;
        // to store a pointer to the latest closest point
        Point *closest_point_;
        ClutterActor *group_;
};

#endif

