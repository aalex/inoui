/**
 * Inoui: A map of sounds.
 */
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <clutter/clutter.h>
#include <cmath>
#include <spatosc/fudi_sender.h>
#include <spatosc/oscreceiver.h>
#include <string>
#include <sstream>
#include <tr1/memory>
#include <vector>
#include <iostream>

#include "avatar.h"
#include "circle.h"
#include "config.h"
#include "grid.h"
#include "map.h"
#include "maths.h"
#include "point.h"
#include "pprint.h"
#include "soundinfo.h"
#include "timer.h"
#include "statesaving.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

namespace fs = boost::filesystem;
namespace po = boost::program_options;

// TODO: use image size to set window size
static const float WINDOW_WIDTH = 1024;
static const float WINDOW_HEIGHT = 768;
static const char *WINDOW_TITLE = "Paysages inouïs d'Orléans ~ Press Escape to quit";
static const gchar *DEFAULT_BACKGROUND_FILE_NAME = "data/orleans_1024x768.png";
static const gint FUDI_SEND_PORT = 14444;
static const std::string FUDI_SEND_HOST = "localhost";
static const std::string OSC_RECEIVE_PORT = "3333";
//static const gint MAP_CENTER_X = 600;
//static const gint MAP_CENTER_Y = 400;
//static const bool MIRROR_FIDUCIAL_POSITION = true;
static const double TIME_BETWEEN_EACH_PLAY = 0.5; // seconds
// FIXME: hard-coding paths is evil
static const std::string DEFAULT_SOUNDS_PREFIX = "/home/kiosk/SONS/";
static const double MAP_X_FROM = -0.1;
static const double MAP_X_TO = 1.1;
static const double MAP_Y_FROM = -0.1;
static const double MAP_Y_TO = 1.1;
static const int SEND_ANGLE_EVERY = 15;
static const double CAMERA_RATIO = 1.0;
static const bool USE_DUMMY_CONTENTS = false;
static const int DEFAULT_FIDUCIAL_ID = 69;
static const std::string DEFAULT_MAP_FILE_NAME = "data/map.xml";

class InouiConfiguration
{
    public:
        InouiConfiguration();
        std::string sound_prefix;
//        unsigned int osc_receive_port;
//        unsigned int fudi_send_port;
//        std::string fudi_send_host;
        std::string background_image;
        int fiducial_id;
        bool horizontal_mirror;
        std::string map_file_name;
        bool use_any_fiducial;
        bool verbose;
        bool vertical_flip;
};

InouiConfiguration::InouiConfiguration() :
    sound_prefix(DEFAULT_SOUNDS_PREFIX),
//    osc_receive_port(OSC_RECEIVE_PORT),
//    fudi_send_port(FUDI_SEND_PORT),
//    fudi_send_host(FUDI_SEND_HOST),
    background_image(DEFAULT_BACKGROUND_FILE_NAME),
    fiducial_id(DEFAULT_FIDUCIAL_ID),
    horizontal_mirror(false),
    map_file_name(DEFAULT_MAP_FILE_NAME),
    use_any_fiducial(false),
    verbose(false),
    vertical_flip(false)
{
    // pass
}

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
        void parse_options(int argc, char *argv[]);
        InouiConfiguration *config() { return config_.get(); }
        bool is_verbose();
        void goto_pos(double x, double y);
        static gboolean on_mouse_button_event(ClutterActor *actor, ClutterButtonEvent *event, gpointer user_data);
    private:
        void add_static_point(gfloat x, gfloat y);
        std::tr1::shared_ptr<Timer> timer_last_played_;
        std::string next_sound_to_play_;
        void load_dummy_contents();
        std::tr1::shared_ptr<InouiConfiguration> config_;
};

bool InouiApplication::is_verbose()
{
    return config()->verbose;
}

Map *InouiApplication::get_map()
{
    return map_.get();
}

InouiApplication::InouiApplication()
{
    config_.reset(new InouiConfiguration);
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
    ClutterActor *image = clutter_texture_new_from_file(config()->background_image.c_str(), &error);
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
    x = inoui::map_double(x, 0.0, CAMERA_RATIO, MAP_X_FROM, MAP_X_TO);
    return x * WINDOW_WIDTH;
}

double map_y_tag_pos_to_pixel(double y)
{
    y = inoui::map_double(y, 0.0, CAMERA_RATIO, MAP_Y_FROM, MAP_Y_TO);
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
        int class_id = (int) argv[2]->i;
        //if (self->is_verbose())
        //    std::cout << "Got class id " << class_id << std::endl;
        if (class_id == self->config()->fiducial_id || self->config()->use_any_fiducial)
        {
            double pos_x = (double) argv[3]->f;
            double pos_y = (double) argv[4]->f;
            //std::cout << "Fiducial is at " << pos_x << "," << pos_y << std::endl;
            if (self->config()->horizontal_mirror)
                pos_x = CAMERA_RATIO - pos_x;
            if (self->config()->vertical_flip)
                pos_y = CAMERA_RATIO - pos_y;
            double mapped_x = map_x_tag_pos_to_pixel(pos_x);
            double mapped_y = map_y_tag_pos_to_pixel(pos_y);
            self->goto_pos(mapped_x, mapped_y);


            double angle = (double) inoui::radians_to_degrees(argv[5]->f);
            self->angle_ = angle;
            // g_print("Fiducial is at (%f, %f). Its angle is %f degrees.\n",
        }
    }
    else
        return 1;
    return 0;
}

void InouiApplication::goto_pos(double x, double y)
{
    update_coords_label(x, y);
    clutter_actor_set_position(avatar_actor,
        x,
        y);

    //Point *closest = 
    get_map()->get_closest_point(x, y);
    //if (closest)
    //{
        // This is done in Map::get_closest_point
        //g_print("Select a point");
        //self->get_map()->set_selected(closest);
    //}
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
        //g_print("on_point_chosen: %s\n", sound_file_name.c_str());
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
            // FIXME: we should use boost::fs to append paths together
            file_name << config()->sound_prefix << next_sound_to_play_;
            // get sound file duration
            // FIXME: getting sndfile duration should be done only once for each file
            long int duration_ms = inoui::get_sound_file_duration(file_name.str());
            message << "play " << file_name.str() << " " << duration_ms << ";\n";
            if (is_verbose())
                g_print("Send FUDI: %s", message.str().c_str()); // no need for endl
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

void InouiApplication::load_dummy_contents()
{
    Point *point = 0;

    Map *the_map = get_map();
    point = the_map->add_point(100.0, 100.0);
    point->add_sound("SR018-quad.wav");
    point->add_sound("SR019-quad.wav");

    point = the_map->add_point(200.0, 200.0);
    point->add_sound("SR020-quad.wav");

    point = the_map->add_point(300.0, 300.00);
    point->add_sound("SR021-quad.wav");
    point->add_sound("SR022-quad.wav");

    point = the_map->add_point(500.0, 500.00);
    point->add_sound("SR021-quad.wav");

    point = the_map->add_point(100.0, 500.00);
    point->add_sound("SR021-quad.wav");

    point = the_map->add_point(800.0, 500.00);
    point->add_sound("SR021-quad.wav");

    point = the_map->add_point(800.0, 200.00);
    point->add_sound("SR021-quad.wav");

    point = the_map->add_point(1000.0, 200.00);
    point->add_sound("SR021-quad.wav");

    point = the_map->add_point(1000.0, 800.00);
    point->add_sound("SR021-quad.wav");

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 6; y++)
        {
            point = the_map->add_point(
                x / 8.0 * WINDOW_WIDTH,
                y / 6.0 * WINDOW_HEIGHT);
            point->add_sound("SR021-quad.wav");
        }
    }
}

gboolean InouiApplication::on_mouse_button_event(ClutterActor* /* actor */, ClutterButtonEvent *event, gpointer user_data)
{
    InouiApplication *app = static_cast<InouiApplication *>(user_data);
    if (event->type == CLUTTER_BUTTON_PRESS)
    {
        //ClutterButtonEvent *button_event = CLUTTER_BUTTON_EVENT(event);
        app->goto_pos(event->x, event->y);
    }
    return TRUE;
}

void InouiApplication::populate_map()
{
    Map *the_map = get_map();
    if (USE_DUMMY_CONTENTS)
    {
        load_dummy_contents();
    }
    else
    {
        if (fs::exists(config()->map_file_name))
            inoui::load_project(the_map, config()->map_file_name, is_verbose());
        else
            std::cout << "Could not find project file " << config()->map_file_name << std::endl;
    }
}

static void on_stage_shown(ClutterActor *stage, gpointer *data)
{
    //g_print("on_stage_shown\n");
    UNUSED(stage);
    InouiApplication *app = (InouiApplication *) data;
    app->reset_timer();
}

/**
 * Parses the command line options.
 */
void InouiApplication::parse_options(int argc, char *argv[])
{
    InouiConfiguration *config = config_.get();

    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Show this help message and exit")
        ("version", "Show program's version number and exit")
        ("verbose,v", po::bool_switch(), "Enables a verbose output")
        ("use-any-fiducial,a", po::bool_switch(), "Tracks every fiducial")
        ("sounds-dir,d", po::value<std::string>()->default_value(config->sound_prefix), "Path to the directory containing the sound files")
        ("image,i", po::value<std::string>()->default_value(config->background_image), "Path to the background image of a map")
        ("mirror,m", po::bool_switch(), "Enables horizontal mirror for the fiducial position")

        ("fiducial-id,f", po::value<int>()->default_value(config->fiducial_id), "Sets the initial fiducial marker to track")
        ("map-file-name,m", po::value<std::string>()->default_value(config->map_file_name), "Path to the XML file for the map")
        ("vertical-flip,V", po::bool_switch(), "Enables vertical flipping for the fiducial position")
        //("osc-receive-port,p", po::value<std::string>(), "Sets the listening OSC port")
        //("fudi-send-port,P", po::value<unsigned int>(), "Sets the port to send FUDI messages to")
        //("fudi-send-addr,a", po::value<std::string>()->default_value("localhost"), "Sets the IP address to send FUDI messages to")
        ;
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);
    
    bool verbose = options["verbose"].as<bool>();
    if (verbose)
        config->verbose = true;
    // Options that makes the program exit:
    if (options.count("help"))
    {
        std::cout << desc << std::endl;
        exit(0);
        //return;
    }
    if (options.count("version"))
    {
        std::cout << PACKAGE << " " << PACKAGE_VERSION << std::endl;
        exit(0);
        //return; 
    }
    // Options to use in the normal way:
    if (options.count("image"))
    {
        config->background_image = options["image"].as<std::string>();
        if (fs::exists(config->background_image))
        {
            if (verbose)
                std::cout << "image is set to " << config->background_image << std::endl;
        }
        else
        {
            std::cout << "Could not find image " << config->background_image << "." << std::endl;
            exit(1); // exit with error
        }
    }
    if (options.count("map-file-name"))
    {
        config->map_file_name = options["map-file-name"].as<std::string>();
        if (fs::exists(config->map_file_name))
        {
            if (verbose)
                std::cout << "map is set to " << config->map_file_name << std::endl;
        }
        else
        {
            std::cout << "Could not find map " << config->map_file_name << "." << std::endl;
            exit(1); // exit with error
        }
    }

    if (options.count("sounds-dir"))
    {
        config->sound_prefix = options["sounds-dir"].as<std::string>();
        if (fs::exists(config->sound_prefix))
        {
            if (verbose)
                std::cout << "sound prefix is set to " << config->sound_prefix << std::endl;
        }
        else
        {
            std::cout << "Could not find sounds dir " << config->sound_prefix << "." << std::endl;
            exit(1); // exit with error
        }
    }

    if (options.count("fiducial-id"))
    {
        config->fiducial_id = options["fiducial-id"].as<int>();
        if (verbose)
            std::cout << "fiducial is set to " << config->fiducial_id << std::endl;
    }

    if (options["mirror"].as<bool>())
    {
        if (verbose)
            std::cout << "Mirror is on" << std::endl;
        config->horizontal_mirror = true;
    }

    if (options["vertical-flip"].as<bool>())
    {
        if (verbose)
            std::cout << "Vertical flip is on" << std::endl;
        config->vertical_flip = true;
    }

    if (options["use-any-fiducial"].as<bool>())
    {
        if (verbose)
            std::cout << "Using any fiducial is on" << std::endl;
        config->use_any_fiducial = true;
    }
}

int main(int argc, char *argv[])
{
    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    ClutterColor avatar_color = { 0x99, 0x00, 0x00, 0x00 };
    InouiApplication app;
    app.parse_options(argc, argv);

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
    app.coord_label_ = clutter_text_new_full("Sans semibold 12px", "Use a fiducial to move the avatar", &text_color);
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

    g_signal_connect(G_OBJECT(app.stage), "button-press-event", G_CALLBACK(InouiApplication::on_mouse_button_event), &app);

    app.fudi_sender.reset(new spatosc::FudiSender("localhost", FUDI_SEND_PORT, false));
    // true for TCP, false for UDP
    clutter_actor_show(stage);
    
    // print some info:
    //inoui::print_actors(app.get_map()->get_actor(), 0);
    g_print("Running inoui.\n");
    clutter_main();
    return 0;
}

