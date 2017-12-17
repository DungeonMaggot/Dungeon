#ifndef DUNGEON_H
#define DUNGEON_H

#include "transformation.h"

#include "utility.h"

#define TILE_RADIUS 1
#define TILE_LENGTH TILE_RADIUS*2

typedef struct v2i
{
    int x;
    int y;
} TilePos;

enum wall_directions
{
    WALL_WEST = 0,
    WALL_EAST,
    WALL_SOUTH,
    WALL_NORTH,

    NUM_WALL_DIRECTIONS
};

enum move_directions
{
    MD_Left,
    MD_Right,
    MD_Backward,
    MD_Forward,

    NUM_MOVE_DIRECTIONS
};

enum relative_column_position
{
    COL_LEFT = 0,
    COL_RIGHT,

    NUM_RELATIVE_COLUMN_POSITIONS
};

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

class DungeonActor;
struct game_state
{
    TilePos PlayerPos;
    Transformation PlayerTransform;
    game_button *NewButtons;
    game_button *OldButtons;
    char *LevelMap;
    int LevelWidth;
    int LevelHeight;
    union
    {
        struct
        {
            DungeonActor *Player;
            DungeonActor *Enemies[5];
        };
        DungeonActor *Entities[6];
    };
};

v2i operator +(v2i &A, v2i &B)
{
    v2i Result = {A.x + B.x, A.y + B.y};

    return Result;
}

v2i operator -(v2i &A, v2i &B)
{
    v2i Result = {A.x - B.x, A.y - B.y};

    return Result;
}

bool operator ==(v2i &A, v2i &B)
{
    bool Result = ((A.x == B.x) && (A.y == B.y));

    return Result;
}

void operator *=(v2i &vec2, float &flt)
{
    vec2.x *= flt;
    vec2.y *= flt;
}

v2i operator -(v2i &A)
{
    v2i Result = A;

    Result.x = -Result.x;
    Result.y = -Result.y;

    return Result;
}


#include <QElapsedTimer>
#include <transformation.h>
#include <idleobserver.h>
#define MOVEMENT_TIME 1.0f
#define ROTATION_TIME 0.7f
#define ACTOR_UP_POS 0.5f
class DungeonActor : public Transformation, public IdleObserver
{
public:
    DungeonActor(int PosX, int PosY, int OrientationX, int OrientationY, game_state *GameState)
        : MoveTimer(0.f), MoveTimerResetValue(MOVEMENT_TIME),
          RotationTimer(0.f), RotationTimerResetValue(ROTATION_TIME),
          TilePosCurrent(v2i{PosX, PosY}),
          OrientationCurrent(v2i{OrientationX, OrientationY}),
          MovementDirection(v2i{0, 0}),
          GameStateRef(GameState)
    {
        this->translate(PosX*TILE_LENGTH, 0.5, PosY*TILE_LENGTH); // set initial position
    }

    bool Move(move_directions MoveDirection)
    {
        bool Result = false;

        // check if movement is currently allowed
        if(   (MoveTimer <= 0.f)
           && (RotationTimer <= 0.f) )
        {
            // calculate target position
            v2i TargetPosCandidate = TilePosCurrent;

            switch(MoveDirection)
            {
                case MD_Left:
                {
                    MovementDirection = {-OrientationCurrent.y, -OrientationCurrent.x};
                } break;

                case MD_Right:
                {
                    MovementDirection = {OrientationCurrent.y, OrientationCurrent.x};
                } break;

                case MD_Backward:
                {
                    MovementDirection = -OrientationCurrent;
                } break;

                case MD_Forward:
                {
                    MovementDirection = OrientationCurrent;
                } break;

                default:
                {} break;
            }

            TargetPosCandidate = TilePosCurrent + MovementDirection;

            // check if target is walkable
            char *TileValue = GameStateRef->LevelMap + TargetPosCandidate.y*GameStateRef->LevelWidth + TargetPosCandidate.x;
            if(*TileValue != ' ')
            {
                // check against other entities
                bool MoveBlockedByEntity = false;
                for(unsigned int EntityIndex = 0;
                    EntityIndex < ArrayCount(GameStateRef->Entities);
                    ++EntityIndex)
                {
                    DungeonActor *a = *(GameStateRef->Entities + EntityIndex);
                    if(    a
                       && (a != this))
                    {
                        if(a->MoveTimer <= 0.f) // other entity is not moving
                        {
                           if(a->TilePosCurrent == this->TilePosTarget)
                           {
                               MoveBlockedByEntity = true;
                               break;
                           }
                        }
                        else // other entity is moving
                        {
                            if(a->TilePosTarget == this->TilePosTarget) // wants to move to same position
                            {
                                MoveBlockedByEntity = true;
                                break;
                            }
                        }
                    }
                }

                if(!MoveBlockedByEntity)
                {
                    this->TilePosTarget = TargetPosCandidate;
                    this->MoveTimer = 2.f;
                    DeltaTimer.start();

                    Result = true;
                }
            }
        }

        return Result;
    }

    void ReadInput() // should be overriden by AI control
    {
        if(GameStateRef->NewButtons[PA_MoveForward].IsPressed)
        {
            Move(MD_Forward);
        }
        if(GameStateRef->NewButtons[PA_MoveBackward].IsPressed)
        {
            Move(MD_Backward);
        }
        if(GameStateRef->NewButtons[PA_MoveLeft].IsPressed)
        {
            Move(MD_Left);
        }
        if(GameStateRef->NewButtons[PA_MoveRight].IsPressed)
        {
            Move(MD_Right);
        }
    }

    void doIt() override
    {
        ReadInput();

        // interpolate movement towards the next grid unit
        if(MoveTimer > 0.f)
        {
            float DeltaTimeSeconds = ((float)(DeltaTimer.restart()))/1000;


            if(MoveTimer < DeltaTimeSeconds)
            {
                DeltaTimeSeconds = MoveTimer;
                TilePosCurrent = TilePosTarget;
                MoveTimer = 0.f;
            }
            else
            {
                MoveTimer -= DeltaTimeSeconds;
            }

            QVector2D Translation = {float(this->MovementDirection.x), float(this->MovementDirection.y)};
            Translation *= (DeltaTimeSeconds / MoveTimerResetValue);

            this->translate(Translation.x(), 0.f, Translation.y());
        }
    }

private:
    QElapsedTimer DeltaTimer;
    float MoveTimer;
    float MoveTimerResetValue;
    float RotationTimer;
    float RotationTimerResetValue;
    v2i TilePosCurrent;
    v2i TilePosTarget;
    v2i OrientationCurrent;
    v2i OrientationTarget;
    v2i MovementDirection;
    game_state *GameStateRef;
};

#endif // DUNGEON_H
