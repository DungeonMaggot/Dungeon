#ifndef INPUT_H
#define INPUT_H

#include "idleobserver.h"
#include "inputregistry.h"

#include "dungeon.h"

void HandleInput(game_button *OldButtons, game_button *NewButtons, player_actions ButtonIndex, bool Pressed)
{
    game_button *Old = OldButtons + ButtonIndex;
    game_button *New = NewButtons + ButtonIndex;
    *New = {}; // clear to zero

    if(Pressed)
    {
        New->IsPressed = true;
        if(!Old->IsPressed)
        {
            New->JustPressed = true;
        }
    }
    else // not pressed
    {
        if(Old->IsPressed)
        {
            New->JustReleased = true;
        }
    }
}

class InputListener : public IdleObserver
{
public:
    InputListener(game_state *GameState)
        : GameStateRef(GameState)
    {
    }

    void doIt() override
    {   
        KeyboardInput* Key = InputRegistry::getInstance().getKeyboardInput();

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_MoveForward,
                    (Key->isKeyPressed('w') && !GameStateRef->DebugCameraActive) || Key->isKeyPressed('i'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_MoveLeft,
                    (Key->isKeyPressed('a') && !GameStateRef->DebugCameraActive) || Key->isKeyPressed('j'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_MoveBackward,
                    (Key->isKeyPressed('s') && !GameStateRef->DebugCameraActive) || Key->isKeyPressed('k'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_MoveRight,
                    (Key->isKeyPressed('d') && !GameStateRef->DebugCameraActive) || Key->isKeyPressed('l'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_RotateLeft, Key->isKeyPressed('q'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_RotateRight, Key->isKeyPressed('e'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_Use, Key->isKeyPressed('f'));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_Attack, Key->isKeyPressed(' '));

        HandleInput(GameStateRef->OldButtons, GameStateRef->NewButtons,
                    PA_SwitchCamera, Key->isKeyPressed('p'));

        game_button *Temp = GameStateRef->OldButtons;
        GameStateRef->OldButtons = GameStateRef->NewButtons;
        GameStateRef->NewButtons = Temp;

        //
        // camera switch happens here
        //
        if(GameStateRef->NewButtons[PA_SwitchCamera].JustPressed)
        {
            DebugCamera *Camera = dynamic_cast<DebugCamera *>(GameStateRef->SceneManagerRef->instance()->getActiveContext()->getCamera());

            if(Camera)
            {
                if(Camera == GameStateRef->PlayerCam) // switch from player cam to debug cam
                {
                    if(GameStateRef->DebugCam)
                    {
                        GameStateRef->SceneManagerRef->instance()->getActiveContext()->setCamera(GameStateRef->DebugCam);
                        GameStateRef->DebugCam->ProcessingInput  = true;

                        GameStateRef->DebugCameraActive = true;
                    }
                }
                else if(Camera == GameStateRef->DebugCam) // switch from debug cam to player cam
                {
                    if(GameStateRef->PlayerCam)
                    {
                        GameStateRef->SceneManagerRef->instance()->getActiveContext()->setCamera(GameStateRef->PlayerCam);
                        GameStateRef->DebugCam->ProcessingInput = false;

                        GameStateRef->DebugCameraActive = false;
                    }
                }
            }

            Player *p = dynamic_cast<Player *>(GameStateRef->Player);
            if(p)
            {
                p->PlayerDrawable->setEnabled(GameStateRef->DebugCameraActive);
            }
        }
    }
private:
    game_state *GameStateRef;

};

#endif // INPUT_H
