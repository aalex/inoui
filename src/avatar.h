#ifndef __AVATAR_H__
#define __AVATAR_H__
#include <clutter/clutter.h>

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

#endif

