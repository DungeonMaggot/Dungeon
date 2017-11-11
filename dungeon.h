#ifndef DUNGEON_H
#define DUNGEON_H

#include "transformation.h"

#define TILE_RADIUS 1
#define TILE_LENGTH TILE_RADIUS*2

typedef struct v2i
{
    int x;
    int y;
} TilePos;

struct game_button
{
    bool IsPressed;
    bool JustPressed;
    bool JustReleased;
    float TimeSinceLastEventMs;
};

enum player_actions
{
    PA_MoveLeft,
    PA_MoveRight,
    PA_MoveBackward,
    PA_MoveForward,
    PA_RotateLeft,
    PA_RotateRight,
    PA_Use,
    PA_Attack,

    // must be last element
    PA_NumActions
};

struct game_state
{
    TilePos PlayerPos;
    Transformation PlayerTransform;
    game_button *NewButtons;
    game_button *OldButtons;
};

#endif // DUNGEON_H
