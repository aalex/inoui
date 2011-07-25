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
        /**
         * Sets the scale of the map.
         * For example, a map might have a scale of 10 m per pixel.
         * TODO: actually use it!
         */
        void set_scale(double scale);
        Point *add_point(ClutterContainer *parent, double x, double y);
        Point *get_closest_point(double x, double y);
        bool set_selected(Point *selected);
        boost::signals2::signal<void (std::string)> point_chosen_signal_;
        // typedefs:
        typedef std::tr1::shared_ptr<Point> PointPtr ;
        typedef std::vector<PointPtr>::iterator PointIterator ;
    private:
        std::vector<std::tr1::shared_ptr<Point> > points_;
        Point *closest_point_;
        double scale_;
};

#endif

