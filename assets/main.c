/**
 * Map of Orleans with hot spots.
 */
#include <clutter/clutter.h>

#define UNUSED(x) ((void) (x))
static const gint WIN_W = 1190;
static const gint WIN_H = 892;
static const gchar *BACKGROUND_FILE_NAME = "orleans_historique.png";
static const gchar *SPOT_FILE_NAME = "spot.png";
static const gint NUM_X = 8;
static const gint NUM_Y = 8;

typedef struct stuff_
{
    ClutterActor *group;
    ClutterActor *spot;
    ClutterActor *stage;
} Stuff;

static void add_point(Stuff *self, gfloat x, gfloat y)
{
    ClutterActor *clone = clutter_clone_new(self->spot);
    clutter_container_add_actor(CLUTTER_CONTAINER(self->group), clone);
    clutter_actor_set_anchor_point_from_gravity(clone, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_position(clone, x, y);
}

static void init_stuff(Stuff *self)
{
    // Background map:
    GError *error = NULL;
    ClutterActor *image = clutter_texture_new_from_file(BACKGROUND_FILE_NAME, &error);
    if (error)
    {
        g_critical("Unable to init image: %s", error->message);
        g_error_free(error);
    }
    else 
        clutter_container_add_actor(CLUTTER_CONTAINER(self->group), image);

    // Init the spot's texture:
    error = NULL;
    self->spot = clutter_texture_new_from_file(SPOT_FILE_NAME, &error);
    if (error)
    {
        g_critical("Unable to init image: %s", error->message);
        g_error_free(error);
    }
    else 
    {
        clutter_container_add_actor(CLUTTER_CONTAINER(self->group), self->spot);
        clutter_actor_hide(self->spot);
    }

    int x;
    int y;
    gfloat x_factor = clutter_actor_get_width(self->stage) / NUM_X;
    gfloat y_factor = clutter_actor_get_height(self->stage) / NUM_Y;
    for (x = 0; x < NUM_X; ++x)
    {
        for (y = 0; y < NUM_Y; ++y)
        {
            add_point(self, 
                x * x_factor + x_factor / 2,
                y * y_factor + y_factor / 2);
        }
    }
}

static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event, gpointer data)
{
    Stuff *self = (Stuff *) data;
    UNUSED(self);
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        case CLUTTER_space:
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    clutter_init(&argc, &argv);

    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    stage = clutter_stage_get_default();
    clutter_stage_set_title(CLUTTER_STAGE(stage), "Press Escape to quit");
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_actor_set_size(stage, WIN_W, WIN_H);

    Stuff *self = g_new0(Stuff, 1);
    self->group = clutter_group_new();

    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb), self);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), self->group);
    
    self->stage = stage;
    init_stuff(self);
    
    clutter_actor_show(stage);
    clutter_main();
    return 0;
}

