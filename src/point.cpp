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
    selected_color_ = clutter_color_new(0xff, 0x00, 0x00, 0x00);
    unselected_color_ = clutter_color_new(0x99, 0x99, 0xff, 0x00);
    // group
    group_ = clutter_group_new();
    clutter_actor_set_name(group_, "point-actor");
    clutter_actor_set_anchor_point_from_gravity(group_, CLUTTER_GRAVITY_CENTER);

    // circle
    circle_ = inoui::create_circle(50.0f, unselected_color_);
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
        if (selected)
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(circle_), selected_color_);
        else
            clutter_rectangle_set_color(CLUTTER_RECTANGLE(circle_), unselected_color_);
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

Point *Point::add_sound(const std::string &name)
{
    sounds_.push_back(name);
    return this;
}

std::string Point::get_next_sound()
{
    unsigned int num_sounds = sounds_.size();
    if (num_sounds == 0)
        return std::string("");
    ++current_;
    if (current_ >= num_sounds)
        current_ = 0;
    return sounds_[current_];
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

std::string Point::get_x_y_as_string()
{
    std::ostringstream os;
    os << "(" << x_ << ", " << y_ << ")";
    return os.str();
}
