#include <sstream>
#include <iostream>
#include "point.h"
#include "circle.h"

Point::Point(double scale, double x, double y) :
    circle_(NULL),
    x_(x),
    y_(y),
    current_(0),
    selected_(false),
    scale_(scale)
{
    // group
    group_ = clutter_group_new();
    clutter_actor_set_name(group_, "point-actor");
    clutter_actor_set_anchor_point_from_gravity(group_, CLUTTER_GRAVITY_CENTER);

    // circle
    ClutterColor circle_color = { 0x99, 0x99, 0xff, 0x00 };
    circle_ = inoui::create_circle(50.0f, &circle_color);
    clutter_actor_set_name(circle_, "point-circle");
    clutter_actor_set_anchor_point_from_gravity(circle_, CLUTTER_GRAVITY_CENTER);
    clutter_container_add_actor(CLUTTER_CONTAINER(group_), circle_);

    // label
    ClutterColor text_color = { 0x00, 0x00, 0x00, 0xff };
    text_ = clutter_text_new_full("Sans semibold 12px", "hello", &text_color);
    clutter_actor_set_name(text_, "point-text");
    clutter_actor_set_anchor_point_from_gravity(text_, CLUTTER_GRAVITY_CENTER);
    clutter_container_add_actor(CLUTTER_CONTAINER(group_), text_);

    set_scale(scale_);
    //update_label();
    set_selected(false);
}

void Point::set_selected(bool selected)
{
    selected_ = selected;
    if (circle_ != NULL)
    {
        ClutterColor yes = { 0xff, 0xcc, 0x33, 0x00 };
        ClutterColor no = { 0x99, 0x99, 0xff, 0x00 };
        if (selected)
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(circle_), &yes);
        else
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(circle_), &no);
    }
}

void Point::set_scale(double scale)
{
    scale_ = scale;
    clutter_actor_set_position(circle_, scale_ * x_, scale_ * y_);
    update_label();
}

ClutterActor *Point::get_actor()
{
    return group_;
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

void Point::update_label()
{
    std::ostringstream os;
    os << "Point (" << x_ << ", " << y_ << ")";
    //std::cout << os.str();
    clutter_text_set_text(CLUTTER_TEXT(text_), os.str().c_str());
}

void Point::set_position(double x, double y)
{
    x_ = x;
    y_ = y;
    // update the actor's position
    set_scale(scale_);
}

