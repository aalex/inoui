/**
 * Clutter spatosc example
 *
 * This example is in the public domain.
 */
#include <algorithm>
#include <clutter/clutter.h>
#include <cmath>
#include <spatosc/spatosc.h>
#include <string>
#include <tr1/memory>
#include <vector>

#include "avatar.h"
#include "circle.h"
#include "grid.h"
#include "map.h"
#include "point.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

static const float WINDOW_WIDTH = 1190;
static const float WINDOW_HEIGHT = 892;
static const char *WINDOW_TITLE = "Press arrow keys to move the sound source";
static const gchar *BACKGROUND_FILE_NAME = "orleans_historique.png";
static const gchar *SPOT_FILE_NAME = "spot.png";
static const gint NUM_X = 8;
static const gint NUM_Y = 8;

/**
 * Info for our little application.
 */
struct ExampleApplication
{
    ClutterActor *avatar_actor;
    ClutterActor *stage;
    ClutterActor *group;
    ClutterActor *spot_texture;
    std::tr1::shared_ptr<spatosc::OscReceiver> osc_receiver;
    std::tr1::shared_ptr<Map> map;
};

static void add_point(ExampleApplication *self, gfloat x, gfloat y)
{
    ClutterActor *clone = clutter_clone_new(self->spot_texture);
    clutter_container_add_actor(CLUTTER_CONTAINER(self->group), clone);
    clutter_actor_set_anchor_point_from_gravity(clone, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_position(clone, x, y);
}

static void init_stuff(ExampleApplication *self)
{
    self->group = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(self->stage), self->group);

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
    self->spot_texture = clutter_texture_new_from_file(SPOT_FILE_NAME, &error);
    if (error)
    {
        g_critical("Unable to init image: %s", error->message);
        g_error_free(error);
    }
    else 
    {
        clutter_container_add_actor(CLUTTER_CONTAINER(self->group), self->spot_texture);
        clutter_actor_hide(self->spot_texture);
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

/**
 * Handler for key events.
 */
static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event,
        gpointer data)
{
    UNUSED(actor);
    UNUSED(data);
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        default:
            break;
    }
}

double radians_to_degrees(double radians)
{
    static double ratio = 180.0 / 3.141592653589793238;
    return radians * ratio;
}

/**
 * Handles /tuio/2Dobj set sessionID classID pos_x pos_y angle vel_X vel_Y vel_Angle motion_acceleration rotation_acceleration
 */
int on_2dobj_received(const char * path, const char * types,
        lo_arg ** argv, int /*argc*/, void * /*data*/, void *user_data)
{   
    UNUSED(path);
    UNUSED(types);
    ExampleApplication *self = static_cast<ExampleApplication*>(user_data);
    if (std::string("set") == reinterpret_cast<const char*>(argv[0]))
    {
        float pos_x = argv[3]->f;
        float pos_y = argv[4]->f;
        // float angle = radians_to_degrees(argv[5]->f);
        // g_print("Fiducial is at (%f, %f). Its angle is %f degrees.\n",
        //     pos_x,
        //     pos_y,
        //     angle);
        clutter_actor_set_position(self->avatar_actor,
            pos_x * clutter_actor_get_width(CLUTTER_ACTOR(self->stage)),
            pos_y * clutter_actor_get_height(CLUTTER_ACTOR(self->stage)));
        Point *closest = self->map.get()->get_closest_point(pos_x, pos_y);
        if (closest)
        {
            self->map.get()->set_selected(closest);
        }
    } else
        return 1;
    return 0;            
}

/**
 * Polls OSC messages on each frame that is drawn.
 */
static void on_paint(ClutterTimeline *timeline, gint msec, gpointer data)
{
    UNUSED(timeline);
    UNUSED(msec);
    ExampleApplication *app = (ExampleApplication *) data;
    while (app->osc_receiver.get()->receive() != 0)
    {}
}

int main(int argc, char *argv[])
{
    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    ClutterColor grid_color = { 0xff, 0xff, 0xff, 0x66 };
    ClutterColor orange = { 0xff, 0xcc, 0x33, 0x00 }; /* transparent orange */
    ExampleApplication app;

    clutter_init(&argc, &argv);
    stage = clutter_stage_get_default();
    app.stage = stage;
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_stage_set_title(CLUTTER_STAGE(stage), WINDOW_TITLE);
    clutter_actor_set_size(stage, WINDOW_WIDTH, WINDOW_HEIGHT);

    inoui::create_grid(CLUTTER_CONTAINER(stage), 10.0f, 10.0f, &grid_color);
    init_stuff(&app);

    app.avatar_actor = inoui::create_circle(50.0f, &orange);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), app.avatar_actor);
    clutter_actor_set_position(app.avatar_actor, WINDOW_WIDTH / 2.0f,
            WINDOW_HEIGHT / 2.0f);
    
    app.osc_receiver.reset(new spatosc::OscReceiver("13333"));
    app.osc_receiver.get()->addHandler("/tuio/2Dobj", "siiffffffff", on_2dobj_received, static_cast<void *>(&app));

    app.map.reset(new Map);
    app.map.get()->add_point(CLUTTER_CONTAINER(stage), 0.0, 0.0);
    app.map.get()->add_point(CLUTTER_CONTAINER(stage), 0.1, 0.1);
    app.map.get()->add_point(CLUTTER_CONTAINER(stage), 0.2, 0.2);
    Point *point = app.map.get()->get_closest_point(0.1, 0.12);
    if (point)
        g_print("Found point at %f %f\n", point->get_x(), point->get_y());

    ClutterTimeline *timeline = clutter_timeline_new(1000);
    clutter_timeline_set_loop(timeline, TRUE);
    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_paint), static_cast<void *>(&app));
    clutter_timeline_start(timeline);
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb),
            static_cast<gpointer>(&app));
    clutter_actor_show(stage);

    clutter_main();
    return 0;
}

