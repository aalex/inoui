#ifndef __POINT_H__
#define __POINT_H__

#include <clutter/clutter.h>
#include <string>
#include <vector>

/**
 * A Point is a location, relative to the reference point, 
 * which contains one or many sounds.
 */
class Point
{
    public:
        Point(ClutterContainer *parent, double x, double y);
        bool add_sound(const std::string &name);
        void set_position(double x, double y);
        std::string get_next_sound();
        double get_x() { return x_; }
        double get_y() { return y_; }
        void set_selected(bool selected);
        bool get_selected() { return selected_; }
    private:
        ClutterActor *actor_;
        std::vector<std::string> sounds_;
        double x_;
        double y_;
        unsigned int current_;
        bool selected_;
};

#endif

