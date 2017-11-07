#ifndef PLANET_H
#define PLANET_H
#include "drawable.h"
#include "simplesphere.h"

class Planet : public Drawable
{
public:
    Planet(): Drawable(new SimpleSphere(3, 10, 10)) {}
    Planet(float R): Drawable(new SimpleSphere(R, 10, 10)) {}
};

#endif // PLANET_H
