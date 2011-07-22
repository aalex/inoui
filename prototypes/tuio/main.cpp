/**
 * Clutter spatosc example
 *
 * This example is in the public domain.
 */
#include <clutter/clutter.h>
#include <spatosc/spatosc.h>
#include <algorithm>
#include <string>
#include <tr1/memory>

static const float WINDOW_WIDTH = 500;
static const float WINDOW_HEIGHT = 500;
static const char *WINDOW_TITLE = "Press arrow keys to move the sound source";

/**
 * Info for our little application.
 */
struct ExampleApplication
{
    ClutterActor *foo_actor;
    spatosc::SoundSource *foo_sound;
    std::tr1::shared_ptr<spatosc::OscReceiver> osc_receiver;
};

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

/**
 * We override the paint method of a rectangle to actually paint a circle.
 *
 * We use its original color, but with an alpha of 100%.
 */
void paint_circle(ClutterActor *actor)
{
    gfloat radius = std::min(clutter_actor_get_width(actor), 
            clutter_actor_get_height(actor)) / 2.0f;
    ClutterColor color;
    clutter_rectangle_get_color(CLUTTER_RECTANGLE(actor), &color);
    cogl_set_source_color4ub(color.red, color.green, color.blue, 0xff);
    cogl_path_arc(radius, radius, radius, radius, 0, 360);
    cogl_path_fill();
}

/**
 * Handler for key events.
 */
static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event,
        gpointer data)
{
    ExampleApplication *app = static_cast<ExampleApplication *>(data);
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        case CLUTTER_Up:
            clutter_actor_move_by(app->foo_actor, 0.0f, -1.0f);
            break;
        case CLUTTER_Down:
            clutter_actor_move_by(app->foo_actor, 0.0f, 1.0f);
            break;
        case CLUTTER_Right:
            clutter_actor_move_by(app->foo_actor, 1.0f, 0.0f);
            break;
        case CLUTTER_Left:
            clutter_actor_move_by(app->foo_actor, -1.0f, 0.0f);
            break;
        default:
            break;
    }
    app->foo_sound->setPosition(clutter_actor_get_x(app->foo_actor),
            clutter_actor_get_y(app->foo_actor), 0);
}

/**
 * Scrolling causes the sound source to move in the z direction.
 */
gboolean pointer_scroll_cb(ClutterActor *actor, ClutterEvent *event,
        gpointer data)
{
    ExampleApplication *app = static_cast<ExampleApplication *>(data);

    ClutterScrollDirection direction;
    direction = clutter_event_get_scroll_direction(event);
    gfloat actor_width;
    gfloat actor_height;
    clutter_actor_get_size(app->foo_actor, &actor_width,
            &actor_height);

    switch (direction)
    {
        case CLUTTER_SCROLL_UP:
            // increase circle radius
            clutter_actor_set_size(app->foo_actor, actor_width * 1.1,
                    actor_height * 1.1);
            // TODO increase sound source's position in z
            break;

        case CLUTTER_SCROLL_DOWN:
            // decrease circle radius
            clutter_actor_set_size(app->foo_actor, actor_width * 0.9f,
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

/**
 * Handles /tuio/2Dobj set sessionID classID pos_x pos_y angle vel_X vel_Y vel_Angle motion_acceleration rotation_acceleration
 */
int on_2dobj_received(const char * path, const char * types,
        lo_arg ** argv, int /*argc*/, void * /*data*/, void *user_data)
{   
    //ExampleApplication *self = static_cast<ExampleApplication*>(user_data);
    if (std::string("set") == reinterpret_cast<const char*>(argv[0]))
    {
        float pos_x = argv[2]->f;
        float pos_y = argv[3]->f;
        float angle = argv[4]->f;
        g_print("Fiducial is at (%f, %f). Its angle is %f degrees.\n",
            pos_x,
            pos_y,
            angle);
    } else
        return 1;
    return 0;            
}

int main(int argc, char *argv[])
{
    using namespace spatosc;
    using std::tr1::shared_ptr;
    Scene scene;
    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    ClutterColor grid_color = { 0xff, 0xff, 0xff, 0x66 };
    ClutterColor orange = { 0xff, 0xcc, 0x33, 0x00 }; /* transparent orange */
    ExampleApplication app;

    clutter_init(&argc, &argv);
    stage = clutter_stage_get_default();
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_stage_set_title(CLUTTER_STAGE(stage), WINDOW_TITLE);
    clutter_actor_set_size(stage, WINDOW_WIDTH, WINDOW_HEIGHT);

    create_grid(CLUTTER_CONTAINER(stage), 10.0f, 10.0f, &grid_color);

    app.foo_actor = clutter_rectangle_new_with_color(&orange);
    g_signal_connect(app.foo_actor, "paint", G_CALLBACK(paint_circle), NULL);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), app.foo_actor);
    clutter_actor_set_anchor_point_from_gravity(app.foo_actor,
            CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_size(app.foo_actor, 50.0f, 50.0f);
    clutter_actor_set_position(app.foo_actor, WINDOW_WIDTH / 2.0f,
            WINDOW_HEIGHT / 2.0f);
    
    // Make it draggable
#if CLUTTER_CHECK_VERSION(1, 4, 0)
    ClutterAction *drag_action = clutter_drag_action_new();
    g_signal_connect(drag_action, "drag-motion", G_CALLBACK(on_drag_motion), 
            static_cast<gpointer>(&app));
    clutter_actor_set_reactive(app.foo_actor, TRUE);
    clutter_actor_add_action(app.foo_actor, drag_action);
#else
    g_print("\nWarning: Dragging disabled. Since it requires Clutter >= 1.4.0\n");
#endif

    app.osc_receiver.reset(new OscReceiver("13333"));
    app.osc_receiver.get()->addHandler("/tuio/2Dobj", "siiffffffff", on_2dobj_received, static_cast<void *>(&app));
    app.foo_sound = scene.createSoundSource("1");
    scene.addTranslator<BasicTranslator>("basic");
    app.foo_sound->setPosition(clutter_actor_get_x(app.foo_actor),
            clutter_actor_get_y(app.foo_actor), 0);
    scene.debugPrint();

    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb),
            static_cast<gpointer>(&app));
    g_signal_connect(stage, "scroll-event", G_CALLBACK(pointer_scroll_cb), 
            static_cast<gpointer>(&app));
            
    clutter_actor_show(stage);

    clutter_main();
    return 0;
}
