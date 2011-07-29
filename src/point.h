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
        Point(double x, double y);
        /**
         * Returns this.
         */
        Point *add_sound(const std::string &name);
        void set_position(double x, double y);
        std::string get_next_sound();
        double get_x() { return x_; }
        double get_y() { return y_; }
        std::string get_x_y_as_string();
        void set_selected(bool selected);
        bool get_selected() { return selected_; }
        ClutterActor *get_actor();
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
        void update_label();
        ClutterColor *selected_color_;
        ClutterColor *unselected_color_;
};

#endif

