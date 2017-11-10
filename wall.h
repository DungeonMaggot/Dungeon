#ifndef WALL_H
#define WALL_H

#include "drawable.h"
#include "trianglemesh.h"
//#include "simpleplane.h"
#include "color.h"

class Wall : public Drawable
{
public:
    Wall(): Drawable(new TriangleMesh("meshes/wall_v2.obj"))
    {

    }

    Wall(QVector4D InputColor): Drawable(new TriangleMesh("meshes/wall_v2.obj"))
    {
        Color *c = this->getProperty<Color>();
        c->setValue(InputColor);
    }
};

#endif // WALL_H
