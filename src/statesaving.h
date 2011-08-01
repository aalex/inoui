#ifndef __STATESAVING_H__
#define __STATESAVING_H__

/**
 * Contains constants for state saving to XML file.
 */

// better name for a constant from libxml2
#define XMLSTR BAD_CAST

namespace statesaving
{
    const char * const ROOT_NODE = "map";
    const char * const FILE_NAME = "project.xml";
    const char * const POINTS_NODE = "clips";
    const char * const POINT_NODE = "clip";
    const char * const POINT_ID_PROPERTY = "id";
    const char * const POINT_X_PROPERTY = "x";
    const char * const POINT_Y_PROPERTY = "y";
    const char * const SOUNDS_NODE = "sounds";
    const char * const SOUND_NODE = "sound";
    const char * const SOUND_NAME_ATTR = "name";
}

#endif

