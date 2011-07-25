#include "pprint.h"

namespace inoui
{

static void on_foreach(ClutterActor *actor, gpointer data)
{
    int indentations = GPOINTER_TO_INT(data);
    print_actors(actor, indentations + 2);
}

void print_actors(ClutterActor *root, int indentations = 0)
{
    int spaces;
    for (spaces = 0; spaces < indentations; ++spaces)
    {
        g_print(" ");
    }
    g_print(" * %s", clutter_actor_get_name(root));
    // g_print(" size=(%f, %f)", clutter_actor_get_width(root), clutter_actor_get_height(root));
    g_print(" pos=(%d, %d)", (int) clutter_actor_get_x(root), (int) clutter_actor_get_y(root));
    g_print("\n");
    if (CLUTTER_IS_CONTAINER(root))
    {
        clutter_container_foreach(CLUTTER_CONTAINER(root), &on_foreach, GINT_TO_POINTER(indentations));
    }
}

};

