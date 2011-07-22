#include "point.h"
#include "circle.h"

void Point::set_selected(bool selected)
{
    selected_ = selected;
    if (actor_ != NULL)
    {
        ClutterColor yes = { 0xff, 0xcc, 0x33, 0x00 };
        ClutterColor no = { 0x99, 0x99, 0xff, 0x00 };
        if (selected)
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(actor_), &yes);
        else
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(actor_), &no);
    }
}

Point::Point(ClutterContainer *parent, double x, double y) :
    actor_(NULL),
    x_(x),
    y_(y),
    current_(0),
    selected_(false)
{
    if (parent != NULL)
    {
        ClutterColor color = { 0xff, 0xcc, 0x33, 0x00 };
        actor_ = inoui::create_circle(50.0f, &color);
        clutter_container_add_actor(parent, actor_);
        clutter_actor_set_position(actor_, clutter_actor_get_width(CLUTTER_ACTOR(parent)) * x_, clutter_actor_get_height(CLUTTER_ACTOR(parent)) * y_);
        set_selected(false);
    }
}

bool Point::add_sound(const std::string &name)
{
    sounds_.push_back(name);
    return true;
}

std::string Point::get_next_sound()
{
    unsigned int length = sounds_.size();
    ++current_;
    if (current_ > length)
        current_ = 0;
    if (length == 0)
        return "";
    else
    {
        return sounds_[current_];
    }
}

void Point::set_position(double x, double y)
{
    clutter_actor_set_position(actor_, x, y);
}

