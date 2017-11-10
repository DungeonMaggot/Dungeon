#ifndef WALL_H
#define WALL_H

#include "drawable.h"
#include "trianglemesh.h"
//#include "simpleplane.h"
//#include "color.h"

class Wall : public Drawable
{
public:
    Wall(QVector4D opt_color = {0.7, 0.7, 0.7, 1.0}): Drawable(new TriangleMesh("meshes/wall_v2.obj"))
    {
        /*Color *c = this->getProperty<Color>();
        c->setValue(opt_color);*/
    }
};

#endif // WALL_H
