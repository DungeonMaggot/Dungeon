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
    {
        m_GameState = GameState;
    }

    void doIt() override
    {
        KeyboardInput* Key = InputRegistry::getInstance().getKeyboardInput();

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_MoveForward, Key->isKeyPressed('w'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_MoveLeft, Key->isKeyPressed('a'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_MoveBackward, Key->isKeyPressed('s'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_MoveRight, Key->isKeyPressed('d'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_RotateLeft, Key->isKeyPressed('q'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_RotateRight, Key->isKeyPressed('e'));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_Use, Key->isKeyPressed(' '));

        HandleInput(m_GameState->OldButtons, m_GameState->NewButtons,
                    PA_Attack, Key->isKeyPressed('f'));

        game_button *Temp = m_GameState->OldButtons;
        m_GameState->OldButtons = m_GameState->NewButtons;
        m_GameState->NewButtons = Temp;
    }
private:
    game_state *m_GameState;
};

#endif // INPUT_H
