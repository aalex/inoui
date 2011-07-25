#include "circle.h"
#include <cmath>
#include <algorithm>

namespace inoui
{

/**
 * We override the paint method of a rectangle to actually paint a circle.
 *
 * We use its original color, but with an alpha of 100%.
 */
static void paint_circle(ClutterActor *actor, gpointer data)
{
    (void) data;
    gfloat radius = std::min(clutter_actor_get_width(actor), 
            clutter_actor_get_height(actor)) / 2.0f;
    ClutterColor color;
    clutter_rectangle_get_color(CLUTTER_RECTANGLE(actor), &color);
    cogl_set_source_color4ub(color.red, color.green, color.blue, 0xff);
    cogl_path_arc(radius, radius, radius, radius, 0, 360);
    cogl_path_fill();
}

ClutterActor *create_circle(float radius, const ClutterColor *color)
{
    ClutterActor *actor = clutter_rectangle_new_with_color(color);
    g_signal_connect(actor, "paint", G_CALLBACK(paint_circle), NULL);
    clutter_actor_set_anchor_point_from_gravity(actor, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_size(actor, radius, radius);
    return actor;
}

};
