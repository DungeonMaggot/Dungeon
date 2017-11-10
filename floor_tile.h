#ifndef FLOOR_TILE_H
#define FLOOR_TILE_H

#include "drawable.h"
#include "trianglemesh.h"
//#include "simpleplane.h"
//#include "color.h"

class FloorTile : public Drawable
{
public:
    // TODO(andreas): Relative path!
    FloorTile(): Drawable(new TriangleMesh("meshes/floor_v2.obj"))
    {
        /*Color *c = this->getProperty<Color>();
        c->setValue(0.5, 0.5, 0.5, 1.0);*/
    }
};

#endif // FLOOR_TILE_H
