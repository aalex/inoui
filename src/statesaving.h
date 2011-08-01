#ifndef __STATESAVING_H__
#define __STATESAVING_H__

class Map;
#include <string>

/**
 * Contains constants for state saving to XML file.
 */

// better name for a constant from libxml2
#define XMLSTR BAD_CAST

namespace inoui
{

bool load_project(Map *map, std::string &file_name);

}; // end of namespace

namespace statesaving
{
    const char * const ROOT_NODE = "map";
    const char * const POINT_NODE = "point";
    const char * const POINT_ID_PROPERTY = "id";
    const char * const POINT_X_PROPERTY = "x";
    const char * const POINT_Y_PROPERTY = "y";
    const char * const SOUND_NODE = "sound";
    const char * const SOUND_NAME_ATTR = "name";
}

#endif

