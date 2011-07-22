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

#include "circle.h"
#include "grid.h"
#include "point.h"
#include "map.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

static const float WINDOW_WIDTH = 500;
static const float WINDOW_HEIGHT = 500;
static const char *WINDOW_TITLE = "Press arrow keys to move the sound source";

/**
 * Info for our little application.
 */
struct ExampleApplication
{
    ClutterActor *avatar_actor;
    ClutterActor *stage;
    std::tr1::shared_ptr<spatosc::OscReceiver> osc_receiver;
    std::tr1::shared_ptr<Map> map;
};

class Avatar
{
    public:
        Avatar(ClutterContainer *parent);
        void set_position(double x, double y);
        void set_orientation(double angle);

    private:
        double x_;
        double y_;
        double angle_;
};

Avatar::Avatar(ClutterContainer *parent)
{
    UNUSED(parent);    
}

/**
 * Handler for key events.
 */
static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event,
        gpointer data)
{
    UNUSED(actor);
    ExampleApplication *app = static_cast<ExampleApplication *>(data);
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        case CLUTTER_Up:
            clutter_actor_move_by(app->avatar_actor, 0.0f, -1.0f);
            break;
        case CLUTTER_Down:
            clutter_actor_move_by(app->avatar_actor, 0.0f, 1.0f);
            break;
        case CLUTTER_Right:
            clutter_actor_move_by(app->avatar_actor, 1.0f, 0.0f);
            break;
        case CLUTTER_Left:
            clutter_actor_move_by(app->avatar_actor, -1.0f, 0.0f);
            break;
        default:
            break;
    }
    //app->foo_sound->setPosition(clutter_actor_get_x(app->avatar_actor),
    //        clutter_actor_get_y(app->avatar_actor), 0);
}

/**
 * Scrolling causes the sound source to move in the z direction.
 */
gboolean pointer_scroll_cb(ClutterActor *actor, ClutterEvent *event,
        gpointer data)
{
    UNUSED(actor);
    ExampleApplication *app = static_cast<ExampleApplication *>(data);

    ClutterScrollDirection direction;
    direction = clutter_event_get_scroll_direction(event);
    gfloat actor_width;
    gfloat actor_height;
    clutter_actor_get_size(app->avatar_actor, &actor_width,
            &actor_height);

    switch (direction)
    {
        case CLUTTER_SCROLL_UP:
            // increase circle radius
            clutter_actor_set_size(app->avatar_actor, actor_width * 1.1,
                    actor_height * 1.1);
            // TODO increase sound source's position in z
            break;

        case CLUTTER_SCROLL_DOWN:
            // decrease circle radius
            clutter_actor_set_size(app->avatar_actor, actor_width * 0.9f,
                    actor_height * 0.9);
            // TODO: decrease sound source's position in z
            break;

        default:
            break;
    }

    return TRUE; /* event has been handled */
}

#if CLUTTER_CHECK_VERSION(1, 4, 0)
static void on_drag_motion( ClutterDragAction *action, ClutterActor *actor,
    gfloat delta_x, gfloat delta_y, gpointer data)
{
    ExampleApplication *app = static_cast<ExampleApplication *>(data);
    (void) app; // Unused
    float x_pos = clutter_actor_get_x(actor) + delta_x;
    float y_pos = clutter_actor_get_y(actor) + delta_y;
    bool stop_it = false;

    if (x_pos >= WINDOW_WIDTH)
    {
        stop_it = true;
        clutter_actor_set_x(actor, WINDOW_WIDTH);
    }
    else if (x_pos <= 0.0f)
    {
        stop_it = true;
        clutter_actor_set_x(actor, 0.0f);
    }
    else if (y_pos >= WINDOW_HEIGHT)
    {
        stop_it = true;
        clutter_actor_set_y(actor, WINDOW_HEIGHT);
    }
    else if (y_pos <= 0.0f)
    {
        stop_it = true;
        clutter_actor_set_y(actor, 0.0);
    }

    // in Clutter 2.0 we will be able to simply return FALSE instead of calling g_signal_stop_emission_by_name
    if (stop_it)
        g_signal_stop_emission_by_name(action, "drag-motion");
}

#endif

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

    app.avatar_actor = inoui::create_circle(50.0f, &orange);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), app.avatar_actor);
    clutter_actor_set_position(app.avatar_actor, WINDOW_WIDTH / 2.0f,
            WINDOW_HEIGHT / 2.0f);
    
    // Make it draggable
#if CLUTTER_CHECK_VERSION(1, 4, 0)
    ClutterAction *drag_action = clutter_drag_action_new();
    g_signal_connect(drag_action, "drag-motion", G_CALLBACK(on_drag_motion), 
            static_cast<gpointer>(&app));
    clutter_actor_set_reactive(app.avatar_actor, TRUE);
    clutter_actor_add_action(app.avatar_actor, drag_action);
#else
    g_print("\nWarning: Dragging disabled. Since it requires Clutter >= 1.4.0\n");
#endif

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
    g_signal_connect(stage, "scroll-event", G_CALLBACK(pointer_scroll_cb), 
            static_cast<gpointer>(&app));
    clutter_actor_show(stage);

    clutter_main();
    return 0;
}

