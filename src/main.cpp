/**
 * Inoui: A map of sounds.
 */
#include <clutter/clutter.h>
#include <cmath>
#include <spatosc/fudi_sender.h>
#include <spatosc/oscreceiver.h>
#include <string>
#include <sstream>
#include <tr1/memory>
#include <vector>

#include "avatar.h"
#include "circle.h"
#include "grid.h"
#include "map.h"
#include "maths.h"
#include "point.h"
#include "pprint.h"
#include "soundinfo.h"
#include "timer.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

static const float WINDOW_WIDTH = 1024;
static const float WINDOW_HEIGHT = 768;
static const char *WINDOW_TITLE = "Paysages inouïs d'Orléans ~ Press Escape to quit";
static const gchar *BACKGROUND_FILE_NAME = "data/orleans_1024x768.png";
static const gint FUDI_SEND_PORT = 14444;
static const std::string OSC_RECEIVE_PORT = "13333";
static const gint MAP_CENTER_X = 600;
static const gint MAP_CENTER_Y = 400;
static const bool MIRROR_FIDUCIAL_POSITION = true;
static const double TIME_BETWEEN_EACH_PLAY = 0.5; // seconds
// FIXME: hard-coding paths is evil
static const std::string SOUNDS_PREFIX = "/home/aalex/SONS/";
static const double MAP_X_FROM = 0.0;
static const double MAP_X_TO = 1.0;
static const double MAP_Y_FROM = 0.0;
static const double MAP_Y_TO = 1.0;
static const int SEND_ANGLE_EVERY = 15;

/**
 * Info for our little application.
 */
class InouiApplication
{
    public:
        InouiApplication();
        // TODO: make data members private
        ClutterActor *avatar_actor;
        ClutterActor *stage;
        ClutterActor *group;
        ClutterActor *spot_texture;
        ClutterActor *coord_label_;
        int angle_sender_counter_;
        std::tr1::shared_ptr<spatosc::OscReceiver> osc_receiver;
        std::tr1::shared_ptr<Map> map_;
        std::tr1::shared_ptr<spatosc::FudiSender> fudi_sender;
        double angle_;
        void on_point_chosen(std::string sound_file_name);
        /**
         * Sends a message to Pd in the form "play <file> <duration>".
         * Messages are sent at a maximum rate of every TIME_BETWEEN_EACH_PLAY seconds.
         * Durations sent to Pd are are in ms.
         */
        void send_play_message_if_needed();
        void send_angle_message();
        void setup_map();
        void init_map_textures();
        void populate_map();
        Map *get_map();
        void reset_timer();
        void update_coords_label(double x, double y);
    private:
        void add_static_point(gfloat x, gfloat y);
        std::tr1::shared_ptr<Timer> timer_last_played_;
        std::string next_sound_to_play_;
};

Map *InouiApplication::get_map()
{
    return map_.get();
}

InouiApplication::InouiApplication()
{
    timer_last_played_.reset(new Timer);
    next_sound_to_play_ = std::string("");
}

void InouiApplication::reset_timer()
{
    timer_last_played_.get()->start();
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

double map_x_tag_pos_to_pixel(double x)
{
    x = inoui::map_double(x, 0.0, 1.0, MAP_X_FROM, MAP_X_TO);
    return x * WINDOW_HEIGHT;
}

double map_y_tag_pos_to_pixel(double y)
{
    y = inoui::map_double(y, 0.0, 1.0, MAP_Y_FROM, MAP_Y_TO);
    return y * WINDOW_HEIGHT;
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
        double pos_x = (double) argv[3]->f;
        double pos_y = (double) argv[4]->f;
        if (MIRROR_FIDUCIAL_POSITION)
        {
            pos_x = 1.0 - pos_x;
        }
        double mapped_x = map_x_tag_pos_to_pixel(pos_x);
        double mapped_y = map_y_tag_pos_to_pixel(pos_y);

        self->update_coords_label(mapped_x, mapped_y);
        clutter_actor_set_position(self->avatar_actor,
            mapped_x,
            mapped_y);

        Point *closest = self->get_map()->get_closest_point(mapped_x, mapped_y);
        if (closest)
        {
            // This is done in Map::get_closest_point
            //g_print("Select a point");
            //self->get_map()->set_selected(closest);
        }

        double angle = (double) inoui::radians_to_degrees(argv[5]->f);
        self->angle_ = angle;
        // g_print("Fiducial is at (%f, %f). Its angle is %f degrees.\n",
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
    app->send_play_message_if_needed();
    app->send_angle_message();
}

void InouiApplication::on_point_chosen(std::string sound_file_name)
{
    if (sound_file_name != "")
    {
        g_print("on_point_chosen: %s\n", sound_file_name.c_str());
        next_sound_to_play_ = sound_file_name;
    }
}

void InouiApplication::send_play_message_if_needed()
{
    Timer *timer = timer_last_played_.get();
    //g_print("Elapsed=%f, next_sound_to_play_=%s\n", timer->elapsed(), next_sound_to_play_.c_str());
    if (next_sound_to_play_ != "")
    {
        if (timer->elapsed() >= TIME_BETWEEN_EACH_PLAY)
        {
            timer->start();
            std::ostringstream message;
            std::ostringstream file_name;
            file_name << SOUNDS_PREFIX << next_sound_to_play_;
            // get sound file duration
            // FIXME: getting sndfile duration should be done only once for each file
            long int duration_ms = inoui::get_sound_file_duration(file_name.str());
            message << "play " << file_name.str() << " " << duration_ms << ";\n";
            g_print("Sending FUDI message: %s \n", message.str().c_str());
            // send sound file info to pd
            fudi_sender.get()->sendFudi(message.str());
            next_sound_to_play_ = "";
        }
    }
}

void InouiApplication::send_angle_message()
{
    ++angle_sender_counter_;
    if ((angle_sender_counter_ % SEND_ANGLE_EVERY) == 0)
    {
        std::ostringstream message;
        message << "angle " << angle_ << ";\n";
        //g_print("Sending FUDI message: %s \n", message.str().c_str());
        fudi_sender.get()->sendFudi(message.str());
    }
}

void InouiApplication::update_coords_label(double x, double y)
{
    std::ostringstream text;
    text << "(" << (int) x << ", " << (int) y << ")";
    clutter_text_set_text(CLUTTER_TEXT(coord_label_), text.str().c_str());
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

    point = the_map->add_point(100.0, 100.0);
    point->add_sound("SR018-quad.wav");
    point->add_sound("SR019-quad.wav");

    point = the_map->add_point(200.0, 200.0);
    point->add_sound("SR020-quad.wav");

    point = the_map->add_point(300.0, 300.00);
    point->add_sound("SR021-quad.wav");
    point->add_sound("SR022-quad.wav");
}

static void on_stage_shown(ClutterActor *stage, gpointer *data)
{
    //g_print("on_stage_shown\n");
    UNUSED(stage);
    InouiApplication *app = (InouiApplication *) data;
    app->reset_timer();
}

int main(int argc, char *argv[])
{
    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    ClutterColor avatar_color = { 0x99, 0x00, 0x00, 0x00 };
    InouiApplication app;

    clutter_init(&argc, &argv);
    stage = clutter_stage_get_default();
    app.stage = stage;
    app.angle_sender_counter_ = 0;
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_stage_set_title(CLUTTER_STAGE(stage), WINDOW_TITLE);
    clutter_actor_set_size(stage, WINDOW_WIDTH, WINDOW_HEIGHT);

    app.group = clutter_group_new();
    clutter_actor_set_name(app.group, "main-group");
    clutter_container_add_actor(CLUTTER_CONTAINER(app.stage), app.group);
    app.init_map_textures();

    app.avatar_actor = inoui::create_circle(25.0f, &avatar_color);
    clutter_actor_set_name(app.avatar_actor, "avatar");
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), app.avatar_actor);
    clutter_actor_set_position(app.avatar_actor, WINDOW_WIDTH / 2.0f,
            WINDOW_HEIGHT / 2.0f);
    
    app.osc_receiver.reset(new spatosc::OscReceiver(OSC_RECEIVE_PORT));
    app.osc_receiver.get()->addHandler("/tuio/2Dobj", "siiffffffff", on_2dobj_received, static_cast<void *>(&app));

    app.setup_map();
    app.populate_map();
    app.angle_ = 0.0;

    //ClutterColor grid_color = { 0x00, 0x00, 0x00, 0x11 };
    //inoui::create_grid(CLUTTER_CONTAINER(stage), 10.0f, 10.0f, &grid_color);

    ClutterColor text_color = { 0xff, 0xff, 0xff, 0xff };
    app.coord_label_ = clutter_text_new_full("Sans semibold 12px", "0x0", &text_color);
    clutter_actor_set_name(app.coord_label_, "coords-text");
    clutter_actor_set_position(app.coord_label_, WINDOW_WIDTH - 200, WINDOW_HEIGHT - 15);
    clutter_container_add_actor(CLUTTER_CONTAINER(app.group), app.coord_label_);

    // Setup a callback that is called on each frame being rendered
    ClutterTimeline *timeline = clutter_timeline_new(1000);
    clutter_timeline_set_loop(timeline, TRUE);
    clutter_timeline_start(timeline);
    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_paint), static_cast<void *>(&app));
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb),
            static_cast<gpointer>(&app));
    g_signal_connect(stage, "show", G_CALLBACK(on_stage_shown),
            static_cast<gpointer>(&app));

    app.fudi_sender.reset(new spatosc::FudiSender("localhost", FUDI_SEND_PORT, false));
    // true for TCP, false for UDP
    clutter_actor_show(stage);
    
    // print some info:
    //inoui::print_actors(app.get_map()->get_actor(), 0);
    //Map *the_map = app.get_map();
    g_print("Running inoui.\n");
    clutter_main();
    return 0;
}

