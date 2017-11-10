#ifndef FLOOR_TILE_H
#define FLOOR_TILE_H

#include "drawable.h"
#include "simpleplane.h"
#include "color.h"

class FloorTile : public Drawable
{
public:
    FloorTile(): Drawable(new SimplePlane(1.f))
    {
        Color *c = this->getProperty<Color>();
        c->setValue(0.5, 0.5, 0.5, 1.0);
    }
};

#endif // FLOOR_TILE_H
