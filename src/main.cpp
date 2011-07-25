/**
 * Inoui: A map of sounds.
 */
#include <clutter/clutter.h>
#include <cmath>
#include <spatosc/fudi_sender.h>
#include <spatosc/oscreceiver.h>
#include <string>
#include <tr1/memory>
#include <vector>

#include "avatar.h"
#include "circle.h"
#include "grid.h"
#include "map.h"
#include "maths.h"
#include "point.h"
#include "pprint.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

static const float WINDOW_WIDTH = 1190;
static const float WINDOW_HEIGHT = 892;
static const char *WINDOW_TITLE = "Press Escape to quit";
static const gchar *BACKGROUND_FILE_NAME = "orleans_historique.png";
static const gchar *SPOT_FILE_NAME = "spot.png";
static const gint NUM_X = 8;
static const gint NUM_Y = 6;
static const gint FUDI_SEND_PORT = 14444;
static const std::string OSC_RECEIVE_PORT = "13333";

/**
 * Info for our little application.
 */
class InouiApplication
{
    public:
        // TODO: make data members private
        ClutterActor *avatar_actor;
        ClutterActor *stage;
        ClutterActor *group;
        ClutterActor *spot_texture;
        std::tr1::shared_ptr<spatosc::OscReceiver> osc_receiver;
        std::tr1::shared_ptr<Map> map_;
        std::tr1::shared_ptr<spatosc::FudiSender> fudi_sender;
        void on_point_chosen(std::string sound_file_name);
        void setup_map();
        void init_map_textures();
        void populate_map();
        Map *get_map();
    private:
        void add_static_point(gfloat x, gfloat y);
};

Map *InouiApplication::get_map()
{
    return map_.get();
}

void InouiApplication::add_static_point(gfloat x, gfloat y)
{
    ClutterActor *clone = clutter_clone_new(spot_texture);
    clutter_actor_set_name(clone, "some-textured-spot");
    clutter_container_add_actor(CLUTTER_CONTAINER(group), clone);
    clutter_actor_set_anchor_point_from_gravity(clone, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_position(clone, x, y);
}

void InouiApplication::init_map_textures()
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
    {
        clutter_actor_set_name(image, "background-image");
        clutter_container_add_actor(CLUTTER_CONTAINER(group), image);
    }

    // Init the spot's texture:
    error = NULL;
    spot_texture = clutter_texture_new_from_file(SPOT_FILE_NAME, &error);
    if (error)
    {
        g_critical("Unable to init image: %s", error->message);
        g_error_free(error);
    }
    else 
    {
        clutter_actor_set_name(image, "spot-texture");
        clutter_container_add_actor(CLUTTER_CONTAINER(group), spot_texture);
        clutter_actor_hide(spot_texture);
    }

    int x;
    int y;
    gfloat x_factor = clutter_actor_get_width(stage) / NUM_X;
    gfloat y_factor = clutter_actor_get_height(stage) / NUM_Y;
    for (x = 0; x < NUM_X; ++x)
    {
        for (y = 0; y < NUM_Y; ++y)
        {
            add_static_point(
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

/**
 * Handles /tuio/2Dobj set sessionID classID pos_x pos_y angle vel_X vel_Y vel_Angle motion_acceleration rotation_acceleration
 */
int on_2dobj_received(const char * path, const char * types,
        lo_arg ** argv, int /*argc*/, void * /*data*/, void *user_data)
{   
    UNUSED(path);
    UNUSED(types);
    InouiApplication *self = static_cast<InouiApplication*>(user_data);
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
        Point *closest = self->get_map()->get_closest_point(pos_x, pos_y);
        if (closest)
        {
            self->get_map()->set_selected(closest);
        }
    }
    else
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
    InouiApplication *app = (InouiApplication *) data;
    while (app->osc_receiver.get()->receive() != 0)
    {
        // pass
    }
}


void InouiApplication::on_point_chosen(std::string sound_file_name)
{
    if (sound_file_name != "")
    {
        //g_print("on_point_chosen: %s\n", sound_file_name.c_str());
        //std::string message = "play " + sound_file_name + ";";
        //g_print("Sending FUDI message: %s \n", message.c_str());
        //FIXME: fudi_sender.get()->sendFudi(message);
    }
}

void InouiApplication::setup_map()
{
    map_.reset(new Map);
    get_map()->point_chosen_signal_.connect(boost::bind(&InouiApplication::on_point_chosen, this, _1));
    clutter_container_add_actor(CLUTTER_CONTAINER(group), get_map()->get_actor());
}

void InouiApplication::populate_map()
{
    Map *the_map = get_map();
    Point *point = 0;

    point = the_map->add_point(300.0, 300.0);
    point->add_sound("a.wav");
    point->add_sound("b.wav");

    point = the_map->add_point(100.0, 100.0);
    point->add_sound("c.wav");

    point = the_map->add_point(200.0, 2.00);
    point->add_sound("d.wav");
}

int main(int argc, char *argv[])
{
    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    ClutterColor grid_color = { 0xff, 0xff, 0xff, 0x66 };
    ClutterColor orange = { 0xff, 0xcc, 0x33, 0x00 }; /* transparent orange */
    InouiApplication app;

    clutter_init(&argc, &argv);
    stage = clutter_stage_get_default();
    app.stage = stage;
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_stage_set_title(CLUTTER_STAGE(stage), WINDOW_TITLE);
    clutter_actor_set_size(stage, WINDOW_WIDTH, WINDOW_HEIGHT);

    inoui::create_grid(CLUTTER_CONTAINER(stage), 10.0f, 10.0f, &grid_color);

    app.group = clutter_group_new();
    clutter_actor_set_name(app.group, "main-group");
    clutter_container_add_actor(CLUTTER_CONTAINER(app.stage), app.group);
    app.init_map_textures();

    app.avatar_actor = inoui::create_circle(50.0f, &orange);
    clutter_actor_set_name(app.avatar_actor, "avatar");
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), app.avatar_actor);
    clutter_actor_set_position(app.avatar_actor, WINDOW_WIDTH / 2.0f,
            WINDOW_HEIGHT / 2.0f);
    
    app.osc_receiver.reset(new spatosc::OscReceiver(OSC_RECEIVE_PORT));
    app.osc_receiver.get()->addHandler("/tuio/2Dobj", "siiffffffff", on_2dobj_received, static_cast<void *>(&app));

    app.setup_map();
    app.populate_map();

    // Setup a callback that is called on each frame being rendered
    ClutterTimeline *timeline = clutter_timeline_new(1000);
    clutter_timeline_set_loop(timeline, TRUE);
    clutter_timeline_start(timeline);
    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_paint), static_cast<void *>(&app));
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb),
            static_cast<gpointer>(&app));

    app.fudi_sender.reset(new spatosc::FudiSender("localhost", FUDI_SEND_PORT, false));

    clutter_actor_show(stage);
    
    // print some info:
    inoui::print_actors(app.get_map()->get_actor(), 0);
    //Map *the_map = app.get_map();
    

    clutter_main();
    return 0;
}

