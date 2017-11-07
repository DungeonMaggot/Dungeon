#ifndef FLOOR_TILE_H
#define FLOOR_TILE_H

#include "drawable.h"
#include "simpleplane.h"

class FloorTile : public Drawable
{
public:
    FloorTile(): Drawable(new SimplePlane(2.f)) {}
};


#endif // FLOOR_TILE_H
