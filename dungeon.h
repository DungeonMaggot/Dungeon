#ifndef DUNGEON_H
#define DUNGEON_H

// SG includes
#include "camera.h"
#include "scenemanager.h"
#include "transformation.h"

// dungeon includes
#include "debug_camera.h"
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

enum rotation_directions
{
    RD_Left,
    RD_Right
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
    PA_SwitchCamera,

    // must be last element
    PA_NumActions
};

class DungeonActor;
struct game_state
{
    SceneManager *SceneManagerRef;
    bool DebugCameraActive;
    DebugCamera *DebugCam;
    Camera *PlayerCam;
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
#define MOVEMENT_TIME 0.5f
#define ROTATION_TIME 0.5f
class DungeonActor : public Transformation, public IdleObserver
{
public:
    DungeonActor(int PosX, int PosY, int DistanceFromFloor, int OrientationX, int OrientationY, game_state *GameState)
        : MoveTimer(0.f), MoveTimerResetValue(MOVEMENT_TIME),
          RotationTimer(0.f), RotationTimerResetValue(ROTATION_TIME),
          TilePosCurrent(v2i{PosX, PosY}),
          OrientationCurrent(v2i{OrientationX, OrientationY}),
          MovementDirection(v2i{0, 0}),
          GameStateRef(GameState)
    {
        this->translate(PosX*TILE_LENGTH, DistanceFromFloor, PosY*TILE_LENGTH); // set initial position
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
                    MovementDirection = {OrientationCurrent.y, OrientationCurrent.x};
                    if(OrientationCurrent.y == 0)
                    {
                        MovementDirection = -MovementDirection;
                    }
                } break;

                case MD_Right:
                {
                    MovementDirection = {OrientationCurrent.y, OrientationCurrent.x};
                    if(OrientationCurrent.x == 0)
                    {
                        MovementDirection = -MovementDirection;
                    }
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
                    AmountMoved = 0.f;
                    this->TilePosTarget = TargetPosCandidate;
                    this->MoveTimer = MoveTimerResetValue;
                    DeltaTimer.start();

                    Result = true;
                }
            }
        }

        return Result;
    }

    bool Rotate(rotation_directions Direction)
    {
        bool Result = false;

        // check if rotation is currently allowed
        if(   (MoveTimer <= 0.f)
           && (RotationTimer <= 0.f) )
        {
            OrientationTarget = {OrientationCurrent.y, OrientationCurrent.x};
            if(   (Direction == RD_Left  && OrientationCurrent.y == 0)
               || (Direction == RD_Right && OrientationCurrent.x == 0) )
            {
                OrientationTarget = -OrientationTarget;
            }

            AmountRotated = 0.f;
            RotationDirection = Direction;
            RotationTimer = RotationTimerResetValue;
            DeltaTimer.start();

            Result = true;
        }

        return Result;
    }


    virtual void Control() = 0;

    void doIt() override
    {
        Control();

        // interpolate movement towards the next grid unit
        bool Moving   = (MoveTimer > 0.f);
        bool Rotating = (RotationTimer > 0.f);
        if(Moving || Rotating)
        {
            float DeltaTimeSeconds = ((float)(DeltaTimer.restart()))/1000;

            if(Moving)
            {
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

                float DistanceToMovePerSecond = TILE_LENGTH/MoveTimerResetValue;
                float DistanceToMoveThisFrame = DistanceToMovePerSecond * DeltaTimeSeconds;

                if((AmountMoved + DistanceToMoveThisFrame) > TILE_LENGTH)
                {
                    DistanceToMoveThisFrame = TILE_LENGTH - AmountMoved;
                }

                QVector2D Translation = {float(this->MovementDirection.x), float(this->MovementDirection.y)};
                Translation *= DistanceToMoveThisFrame;

                this->translate(Translation.x(), 0.f, Translation.y());
                AmountMoved += DistanceToMoveThisFrame;
            }

            if(Rotating)
            {
                if(RotationTimer < DeltaTimeSeconds)
                {
                    DeltaTimeSeconds = RotationTimer;
                    OrientationCurrent = OrientationTarget;
                    RotationTimer = 0.f;
                }
                else
                {
                    RotationTimer -= DeltaTimeSeconds;
                }

                float DegreesToRotatePerSecond = 90.f/RotationTimerResetValue;
                float AngleForThisFrame = DegreesToRotatePerSecond * DeltaTimeSeconds;

                if((AmountRotated + AngleForThisFrame) > 90.f)
                {
                    AngleForThisFrame = 90 - AmountRotated;
                }

                if(RotationDirection == RD_Left)
                {
                    AngleForThisFrame = -AngleForThisFrame;
                }

                GameStateRef->PlayerCam->rotate(AngleForThisFrame, 0.f, 0.f);
                AmountRotated += AngleForThisFrame;
            }
        }
    }

protected:
    QElapsedTimer DeltaTimer;
    float MoveTimer;
    float MoveTimerResetValue;
    float RotationTimer;
    float RotationTimerResetValue;
    float AmountMoved;
    float AmountRotated;
    v2i TilePosCurrent;
    v2i TilePosTarget;
    v2i OrientationCurrent;
    v2i OrientationTarget;
    v2i MovementDirection;
    rotation_directions RotationDirection;
    game_state *GameStateRef;
};

#include "drawable.h"
class Player : public DungeonActor
{
public:

    Player(int PosX, int PosY, int DistanceFromFloor, int OrientationX, int OrientationY, game_state *GameState)
        : DungeonActor(PosX, PosY, DistanceFromFloor, OrientationX, OrientationY, GameState)
    {}

    void Control() override // should be overriden by AI control
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

        if(GameStateRef->NewButtons[PA_RotateLeft].IsPressed)
        {
            Rotate(RD_Left);
        }
        if(GameStateRef->NewButtons[PA_RotateRight].IsPressed)
        {
            Rotate(RD_Right);
        }
    }

    void doIt() override
    {
        // update camera - move this into player
        QVector4D PlayerPos = this->getModelMatrix().column(3);
        GameStateRef->PlayerCam->setPosition(PlayerPos.toVector3D());

        DungeonActor::doIt();
    }

    Drawable *PlayerDrawable;
};

#endif // DUNGEON_H
