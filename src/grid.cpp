#include "grid.h"

namespace inoui
{

/**
 * Draws a grid.
 *
 * The parent must have a size.
 * Note that it adds it to its parent.
 */
ClutterActor *create_grid(ClutterContainer *parent, gfloat interval_x, gfloat interval_y, ClutterColor *color)
{
    // group
    ClutterActor *group = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(parent), CLUTTER_ACTOR(group));
    gfloat parent_w = clutter_actor_get_width(CLUTTER_ACTOR(parent));
    gfloat parent_h = clutter_actor_get_height(CLUTTER_ACTOR(parent));

    gfloat x, y;
    ClutterActor *rect = NULL;
    // vertical lines:
    for (x = 0.0; x < parent_w; x += interval_x)
    {
        rect = clutter_rectangle_new_with_color(color);
        clutter_container_add_actor(CLUTTER_CONTAINER(group), CLUTTER_ACTOR(rect));
        clutter_actor_set_height(rect, parent_h);
        clutter_actor_set_width(rect, 1.0);
        clutter_actor_set_position(rect, x, 0.0);
    }
    for (y = 0.0; y < parent_h; y += interval_y)
    {
        rect = clutter_rectangle_new_with_color(color);
        clutter_container_add_actor(CLUTTER_CONTAINER(group), CLUTTER_ACTOR(rect));
        clutter_actor_set_height(rect, 1.0);
        clutter_actor_set_width(rect, parent_w);
        clutter_actor_set_position(rect, 0.0, y);
    }
    return group;
}

};
