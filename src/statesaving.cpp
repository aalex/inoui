// #include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <cstdlib> // for getenv
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h> // for snprintf
#include <string>

#include "point.h"
#include "map.h"
#include "statesaving.h"

namespace
{
    bool node_name_is(xmlNode *node, const std::string &name)
    {
        return (node->type == XML_ELEMENT_NODE && node->name && (xmlStrcmp(node->name, XMLSTR name.c_str()) == 0));
    }

    /** Returns a pointer to the XML child with the given name
     * @return A pointer to the data, not a copy of it.
     */
    xmlNode *seek_child_named(xmlNode *parent, const std::string &name)
    {
        if (parent == NULL)
            return NULL;
        for (xmlNode *node = parent->children; node != NULL; node = node->next)
        {
            if (node_name_is(node, name))
            {
                return node;
            }
        }
        return NULL;
    }
} // end of namespace

namespace inoui
{

bool load_project(Map *map, std::string &file_name)
{
    namespace ss = statesaving;
    bool verbose = true;
    xmlDoc *doc = xmlReadFile(file_name.c_str(), NULL, 0);

    if (doc == NULL)
    {
        printf("error: could not parse file %s\n", file_name.c_str());
        return false;
    }
    else if (verbose)
        std::cout << "Loading project file " << file_name << std::endl;

    xmlNode *root = xmlDocGetRootElement(doc);

    // POINTS:
    xmlNode *points_node = seek_child_named(root, ss::POINTS_NODE);
    if (points_node != NULL)
    {
        for (xmlNode *point_node = points_node->children; point_node; point_node = point_node->next)
        {
            std::string point_id_str = "";
            double point_pos_x = 0.0;
            double point_pos_y = 0.0;
            Point *point = 0;

            // point:
            if (node_name_is(point_node, ss::POINT_NODE))
            {
                // Node id:
                xmlChar *point_id = xmlGetProp(point_node, XMLSTR ss::POINT_ID_PROPERTY);
                if (point_id != NULL)
                {
                    if (verbose)
                        printf(" * Point ID: %s\n", point_id);
                    point_id_str = std::string((char *) point_id);
                }
                xmlFree(point_id); // free the property string

                // X LOCATION:
                xmlChar *x_prop = xmlGetProp(point_node, XMLSTR ss::POINT_X_PROPERTY);
                if (x_prop != NULL)
                {
                    try
                    {
                        point_pos_x = boost::lexical_cast<double>(x_prop);
                        if (verbose)
                            printf("Point x: %f \n", point_pos_x);
                    }
                    catch (boost::bad_lexical_cast &)
                    {
                        g_critical("Invalid double for %s in XML file: %s", x_prop, file_name.c_str());
                    }
                }
                xmlFree(x_prop); // free the property string

                // Y LOCATION:
                xmlChar *y_prop = xmlGetProp(point_node, XMLSTR ss::POINT_Y_PROPERTY);
                if (y_prop != NULL)
                {
                    try
                    {
                        point_pos_y = boost::lexical_cast<double>(y_prop);
                        if (verbose)
                            printf("Point y: %f \n", point_pos_y);
                    }
                    catch (boost::bad_lexical_cast &)
                    {
                        g_critical("Invalid double for %s in XML file: %s", y_prop, file_name.c_str());
                    }
                }
                xmlFree(y_prop); // free the property string
                
                point = map->add_point(point_pos_x, point_pos_y);

                for (xmlNode *sound_node = point_node->children; sound_node; sound_node = sound_node->next)
                {
                    // sound:
                    if (node_name_is(sound_node, ss::SOUND_NODE))
                    {
                        // sound file name
                        xmlChar *sound_name = xmlGetProp(sound_node, XMLSTR ss::SOUND_NAME_ATTR);
                        if (sound_name != NULL)
                        {
                            if (verbose)
                                printf(" * Sound name: %s\n", sound_name);
                            point->add_sound((char *) sound_name);
                        } // sound name attribute
                        xmlFree(sound_name); // free the property string
                    } // if SOUND_NODE
                } // for each sound
            } // if POINT_node
        } // end of for each point
    } // end of points node
    // Free the document + global variables that may have been allocated by the parser.
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return true;
}

}; // end of namespace

