#ifndef DUNGEON_H
#define DUNGEON_H

// C includes
#include <stdlib.h>

// SG includes
#include "camera.h"
#include "scenemanager.h"
#include "transformation.h"
#include "audioengine.h"
#include "soundsource.h"

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

enum dungeon_actor_state
{
    DAS_Moving,
    DAS_Rotating,
    DAS_Attacking,
    DAS_Waiting,
    DAS_AwaitingControl
};

enum sound_effects
{
    SFX_Step,
    SFX_Attack,
    SFX_TakeDamage,

    SFX_NUM_SOUND_EFFECTS
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
    SoundFile *Sound_PlayerStep;
    SoundFile *Sound_PlayerAttack;
    SoundFile *Sound_PlayerTakeDamage;
    SoundFile *Sound_PlayerDie;
    SoundFile *Sound_EnemyStep;
    SoundFile *Sound_EnemyAttack;
    SoundFile *Sound_EnemyTakeDamage;
    SoundFile *Sound_EnemyDie;
};

v2i operator +(v2i A, v2i B)
{
    v2i Result = {A.x + B.x, A.y + B.y};

    return Result;
}

void operator +=(v2i &A, v2i &B)
{
    A = A + B;
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

bool operator !=(v2i &A, v2i &B)
{
    bool Result = !((A.x == B.x) && (A.y == B.y));

    return Result;
}

v2i operator *(float &flt, v2i &vec2)
{
    v2i Result = vec2;
    vec2.x *= flt;
    vec2.y *= flt;

    return Result;
}

v2i operator *(v2i vec2, float flt)
{
    v2i Result = flt*vec2;

    return Result;
}

void operator *=(v2i &vec2, float &flt)
{
    vec2 = flt*vec2;
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
#define ATTACK_TIME   0.5f
#define PLAYER_HITPOINTS 10
#define ENEMY_HITPOINTS   5
class DungeonActor : public Transformation, public IdleObserver
{
public:
    DungeonActor(int PosX, int PosY, float DistanceFromFloor, int OrientationX, int OrientationY, game_state *GameState)
        : Timer(0.f),
          MoveTimerResetValue(MOVEMENT_TIME),
          RotationTimerResetValue(ROTATION_TIME),
          AttackTimerResetValue(ATTACK_TIME),
          State(DAS_AwaitingControl),
          Hitpoints(1),
          TilePosCurrent(v2i{PosX, PosY}),
          OrientationCurrent(v2i{OrientationX, OrientationY}),
          MovementDirection(v2i{0, 0}),
          GameStateRef(GameState)
    {
        this->translate(PosX*TILE_LENGTH, DistanceFromFloor, PosY*TILE_LENGTH); // set initial position
    }

    ~DungeonActor()
    {
        for(int Index = 0; Index < SFX_NUM_SOUND_EFFECTS; ++Index)
        {
            if(Sounds[Index])
            {
                Sounds[Index]->stop();
                delete Sounds[Index];
            }
        }
    }

    bool Move(move_directions MoveDirection)
    {
        bool Result = false;

        // check if movement is currently allowed
        if(State == DAS_AwaitingControl)
        {   
            // calculate movement direction
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

            v2i TargetPosCandidate = TilePosCurrent + MovementDirection;

            // check if target is outside of the level boundaries
            if(   (TargetPosCandidate.x >= 0)
               && (TargetPosCandidate.x <  GameStateRef->LevelWidth)
               && (TargetPosCandidate.y >= 0)
               && (TargetPosCandidate.y <  GameStateRef->LevelHeight) )
            {
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
                           && (a != this)
                           && (a->Hitpoints > 0))
                        {
                            if(a->Timer <= 0.f) // other entity is not moving
                            {
                               if(a->TilePosCurrent == TargetPosCandidate) // occupies desired target
                               {
                                   MoveBlockedByEntity = true;
                                   break;
                               }
                            }
                            else // other entity is moving
                            {
                                if(a->TilePosTarget == TargetPosCandidate) // wants to move to same position
                                {
                                    MoveBlockedByEntity = true;
                                    break;
                                }
                            }
                        }
                    }

                    if(!MoveBlockedByEntity)
                    {
                        State = DAS_Moving;
                        AmountMoved = 0.f;
                        this->TilePosTarget = TargetPosCandidate;
                        this->Timer = MoveTimerResetValue;

                        Result = true;
                    }
                }
            }
        }

        return Result;
    }

    virtual void MoveStartEvent()
    {
        Sounds[SFX_Step]->play();
    }

    bool Rotate(rotation_directions Direction)
    {
        bool Result = false;

        // check if rotation is currently allowed
        if(State == DAS_AwaitingControl)
        {
            OrientationTarget = {OrientationCurrent.y, OrientationCurrent.x};
            if(   (Direction == RD_Left  && OrientationCurrent.y == 0)
               || (Direction == RD_Right && OrientationCurrent.x == 0) )
            {
                OrientationTarget = -OrientationTarget;
            }

            State = DAS_Rotating;
            AmountRotated = 0.f;
            RotationDirection = Direction;
            Timer = RotationTimerResetValue;

            Result = true;
        }

        return Result;
    }

    virtual void RotationDoneUpdate(rotation_directions Direction) = 0;

    bool Attack()
    {
        bool Result = false;

        // check if attacking is currently allowed
        if(State == DAS_AwaitingControl)
        {
            Sounds[SFX_Attack]->play();

            State = DAS_Attacking;
            Timer = AttackTimerResetValue;

            AttackTargetTile = TilePosCurrent + (OrientationCurrent * TILE_LENGTH);

            Result = true;
        }

        return Result;
    }

    virtual void AttackAnimationStep(float DeltaTime) = 0;
    virtual void AttackAnimationReset() = 0;

    void ApplyDamage(int Damage)
    {
        Sounds[SFX_TakeDamage]->play();

        Hitpoints -= Damage;
        if(Hitpoints <= 0)
        {
            Die();
        }
    }

    virtual void Die() = 0;


    virtual void Control() = 0; // overridden by player/AI control

    void doIt() override
    {
        Control();

        float DeltaTimeSeconds = ((float)(DeltaTimer.restart()))/1000;
        Update(DeltaTimeSeconds);
    }

    virtual void Update(float DeltaTimeSeconds)
    {
        if(DeltaTimeSeconds > 0.f) // QElapsedTime may pass negative values
        {

            switch(State)
            {
                case DAS_Moving:
                {
                    InterpolateMovement(DeltaTimeSeconds);
                } break;

                case DAS_Rotating:
                {
                    InterpolateRotation(DeltaTimeSeconds);
                } break;

                case DAS_Attacking:
                {
                    InterpolateAttack(DeltaTimeSeconds);
                } break;

                case DAS_Waiting:
                {
                    Wait(DeltaTimeSeconds);
                } break;

                case DAS_AwaitingControl:
                default:
                { } break;
            }
        }
    }

    void InterpolateMovement(float DeltaTimeSeconds)
    {
        if(Timer > 0.f) // currently moving
        {
            if(Timer < DeltaTimeSeconds)
            {
                DeltaTimeSeconds = Timer;
                TilePosCurrent = TilePosTarget;
                Timer = 0.f;
                State = DAS_AwaitingControl;
            }
            else
            {
                Timer -= DeltaTimeSeconds;
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
    }

    virtual void RotationUpdate(float AngleForThisFrame) =0;

    void InterpolateRotation(float DeltaTimeSeconds)
    {
        if(Timer > 0.f) // currently rotating
        {
            if(Timer < DeltaTimeSeconds)
            {
                DeltaTimeSeconds = Timer;
                OrientationCurrent = OrientationTarget;
                Timer = 0.f;
                State = DAS_AwaitingControl;

                RotationDoneUpdate(RotationDirection);
            }
            else
            {
                Timer -= DeltaTimeSeconds;
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

            AmountRotated += AngleForThisFrame;

            // used for player camera update / enemy rotation
            RotationUpdate(AngleForThisFrame);
        }
    }

    void InterpolateAttack(float DeltaTimeSeconds)
    {
        if(Timer > 0.f) // currently attacking
        {
            static bool AttackTriggered;

            if(Timer < DeltaTimeSeconds)
            {
                DeltaTimeSeconds = Timer;
                Timer = 0.f;
                AttackTriggered = false;
                AttackAnimationReset();
                State = DAS_AwaitingControl;
            }
            else
            {
                Timer -= DeltaTimeSeconds;
            }

            AttackAnimationStep(DeltaTimeSeconds);

            if(   !AttackTriggered
               && (Timer <= (AttackTimerResetValue*0.5f)) )
            {
                AttackTriggered = true;

                for(unsigned int EntityIndex = 0;
                    EntityIndex < ArrayCount(GameStateRef->Entities);
                    ++EntityIndex)
                {
                    DungeonActor *a = *(GameStateRef->Entities + EntityIndex);
                    if(    a
                       && (a->Hitpoints > 0)
                       && (a != this) )
                    {
                        if(a->TilePosCurrent == this->AttackTargetTile)
                        {
                            a->ApplyDamage(1);
                        }
                    }
                }
            }
        }
    }

    void Wait(float DeltaTime)
    {
        // only used by AI
        Timer -= DeltaTime;
        if(Timer < 0)
        {
            Timer = 0;
            State = DAS_AwaitingControl;
        }
    }

    Drawable *ActorModel;

    QElapsedTimer DeltaTimer;
    float Timer;
    float MoveTimerResetValue;
    float RotationTimerResetValue;
    float AttackTimerResetValue;
    float AmountMoved;
    float AmountRotated;
    int Hitpoints;
    dungeon_actor_state State;
    v2i TilePosCurrent;
    v2i TilePosTarget;
    v2i OrientationCurrent;
    v2i OrientationTarget;
    v2i MovementDirection;
    v2i AttackTargetTile;
    rotation_directions RotationDirection;
    SoundSource *Sounds[SFX_NUM_SOUND_EFFECTS];
    game_state *GameStateRef;
};

#include "drawable.h"
class Player : public DungeonActor
{
public:

    Player(int PosX, int PosY, float DistanceFromFloor, int OrientationX, int OrientationY, game_state *GameState)
        : DungeonActor(PosX, PosY, DistanceFromFloor, OrientationX, OrientationY, GameState)
    {
        Hitpoints = PLAYER_HITPOINTS;

        // setup audio
# if 1
        Sounds[SFX_Step]       = new SoundSource(GameStateRef->Sound_PlayerStep);
        Sounds[SFX_Attack]     = new SoundSource(GameStateRef->Sound_PlayerAttack);
        Sounds[SFX_TakeDamage] = new SoundSource(GameStateRef->Sound_PlayerTakeDamage);
# endif
    }

    void Control() override
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

        if(GameStateRef->NewButtons[PA_Attack].IsPressed)
        {
            Attack();
        }
    }

    void Die() override
    {

    }

    virtual void RotationUpdate(float AngleForThisFrame) override
    {
        // TODO(andreas): Move PlayerCam into Player
        GameStateRef->PlayerCam->rotate(AngleForThisFrame, 0.f, 0.f);
    }

    virtual void RotationDoneUpdate(rotation_directions Direction) override
    {
        float Angle = (Direction == RD_Left) ? 90.f : -90.f ;
        WeaponPivot->rotate(Angle, 0.f, 1.f, 0.f);
    }

    virtual void AttackAnimationStep(float DeltaTime) override
    {
        float DegreesToRotatePerSecond = 60.f/AttackTimerResetValue;
        float AngleForThisFrame = -(DegreesToRotatePerSecond * DeltaTime);

        Weapon->rotate(AngleForThisFrame, 1.f, 0.f, 0.f);
    }

    virtual void AttackAnimationReset() override
    {
        Weapon->rotate(60.f, 1.f, 0.f, 0.f);
    }

    void doIt() override
    {
        QVector4D PlayerPos = this->getModelMatrix().column(3);
        GameStateRef->PlayerCam->setPosition(PlayerPos.toVector3D());

        DungeonActor::doIt();
    }

    Drawable *PlayerDrawable;
    Transformation *WeaponPivot;
    Transformation *Weapon;
};

class Megaskull : public DungeonActor
{
public:
    Megaskull(int PosX, int PosY, float DistanceFromFloor, int OrientationX, int OrientationY, game_state *GameState)
        : DungeonActor(PosX, PosY, DistanceFromFloor, OrientationX, OrientationY, GameState)
    {
        Hitpoints = ENEMY_HITPOINTS;
        LastKnownPlayerPosition = {-1, -1};
        WaitTimerResetValue = 3.0f;
        PreviousChoice = 0;

        // setup audio
#if 1
        Sounds[SFX_Step]       = new SoundSource(GameStateRef->Sound_EnemyStep);
        Sounds[SFX_Attack]     = new SoundSource(GameStateRef->Sound_EnemyAttack);
        Sounds[SFX_TakeDamage] = new SoundSource(GameStateRef->Sound_EnemyTakeDamage);
#endif
    }

    void Control() override
    {
        if(State == DAS_AwaitingControl)
        {
            // test if player is in sight
            bool PlayerInSight = false;
            int DistanceToPlayer = 0;
            v2i TestPosition = {};
            for(TestPosition = TilePosCurrent + OrientationCurrent;
                   (TestPosition.x >= 0)
                && (TestPosition.x  < GameStateRef->LevelWidth)
                && (TestPosition.y >= 0)
                && (TestPosition.y  < GameStateRef->LevelWidth);
                TestPosition += OrientationCurrent,
                ++DistanceToPlayer)
            {
                char *c = GameStateRef->LevelMap + TestPosition.y*GameStateRef->LevelWidth + TestPosition.x;
                if(*c == ' ')
                {
                    break;
                }
                if(TestPosition == GameStateRef->Player->TilePosCurrent)
                {
                    PlayerInSight = true;
                    LastKnownPlayerPosition = {-1, -1};
                    break;
                }
            }

            if(PlayerInSight)
            {
                if(DistanceToPlayer == 0) // player in attack range
                {
                    Attack();
                }
                else
                {
                    Move(MD_Forward);
                }
            }
            else // player not in sight
            {
                // move towards last known player position
                if(   (LastKnownPlayerPosition != TilePosCurrent)
                   && (LastKnownPlayerPosition.x != -1)
                   && (LastKnownPlayerPosition.y != -1) ) // player is visible
                {
                    Move(MD_Forward);
                }
                else // is at last known player position, or player hasn't been seen yet
                {
                    if(State != DAS_Waiting)
                    {
                        int Choice;
                        if(PreviousChoice > 0)
                        {
                            Choice = 0;
                        }
                        else
                        {
                            Choice = rand() % 3;
                        }
                        PreviousChoice = Choice;

                        switch(Choice)
                        {
                            case 0:
                            {
                                State = DAS_Waiting;
                                Timer = WaitTimerResetValue;
                            } break;

                            case 1:
                            {
                                Rotate(RD_Left);
                            } break;

                            case 2:
                            {
                                Rotate(RD_Right);
                            } break;

                            default:
                            { } break;
                        }
                    }
                }
            }
        }
    }

    void Die() override
    {
        if(ActorModel)
        {
            ActorModel->setEnabled(false);
        }
    }

    virtual void RotationUpdate(float AngleForThisFrame) override
    {
        rotate(AngleForThisFrame, 0.f, 1.f, 0.f);
    }

    virtual void RotationDoneUpdate(rotation_directions Direction) override
    {

    }

    virtual void AttackAnimationStep(float DeltaTime) override
    {

    }

    virtual void AttackAnimationReset() override
    {

    }

protected:
    v2i LastKnownPlayerPosition;
    float WaitTimerResetValue;
    int PreviousChoice = 0;
};

#endif // DUNGEON_H
