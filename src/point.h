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
        Point(double scale, double x, double y);
        bool add_sound(const std::string &name);
        void set_position(double x, double y);
        std::string get_next_sound();
        double get_x() { return x_; }
        double get_y() { return y_; }
        void set_selected(bool selected);
        bool get_selected() { return selected_; }
        ClutterActor *get_actor();
        void set_scale(double scale);
    private:
        // actors are not destructed when this object is destroyed
        ClutterActor *circle_;
        ClutterActor *group_;
        ClutterActor *text_;
        std::vector<std::string> sounds_;
        double x_;
        double y_;
        unsigned int current_;
        bool selected_;
        double scale_;
        void update_label();
};

#endif

