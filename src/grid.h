#ifndef __GRID_H__
#define __GRID_H__
#include <clutter/clutter.h>

namespace inoui
{

ClutterActor *create_grid(ClutterContainer *parent, gfloat interval_x, gfloat interval_y, ClutterColor *color);

};

#endif
