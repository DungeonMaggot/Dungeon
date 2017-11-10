#ifndef INPUT_H
#define INPUT_H

#include <QElapsedTimer>
#include <QWidget>

#include "idleobserver.h"
#include "listener.h"

#include "dungeon.h"

#define RELEASE_THRESHOLD 1000 // ms

class KeyboardGrabber : public QWidget
{
    void keyPressEvent(QKeyEvent *event)
    {
        switch(event->key())
        {
        case Qt::Key_W:
        {} break;

        case Qt::Key_A:
        {} break;

        case Qt::Key_S:
        {} break;

        case Qt::Key_D:
        {} break;

        default: {} break;
        }
    }
};

void HandleInput(game_button *Button)
{
    Button->IsPressed = true;
      printf("Button ? pressed.\n");
    Button->JustReleased = false;
    if(Button->TimeSinceLastEventMs >= RELEASE_THRESHOLD)
    {
        Button->JustPressed = true;
        printf("Button ? just pressed.\n");
    }
    Button->TimeSinceLastEventMs = 0;
}

class InputListener : public Listener, public IdleObserver
{
public:
    InputListener(game_button *Buttons)
    {
        m_Buttons = Buttons;
        m_PreviousTimeNs = 0;
        m_Timer.start();
    }

    void doIt()
    {
        quint64 NewTimeNs = m_Timer.nsecsElapsed();
        float DeltaTimeMs = (float)(NewTimeNs - m_PreviousTimeNs) / 1000000;

        for(int i = 0; i < PA_NumActions; ++i)
        {
            game_button *Button = (m_Buttons + i);
            float OldTimeValue = Button->TimeSinceLastEventMs;
            float NewTimeValue = (OldTimeValue + DeltaTimeMs);

            if(Button->JustPressed)
            {
                Button->JustPressed = false;
                 printf("Button %d un-\"just pressed\".\n", Button - m_Buttons);
            }

            if(   (Button->IsPressed)
               && (OldTimeValue < RELEASE_THRESHOLD)
               && (NewTimeValue >= RELEASE_THRESHOLD) )
            {
                Button->IsPressed = false;
                Button->JustReleased = true;

                printf("Button %d released.\n", Button - m_Buttons);
            }
            else if(Button->JustReleased)
            {
                Button->JustReleased = false;
                 printf("Button %d un-released.\n", Button - m_Buttons);
            }

            Button->TimeSinceLastEventMs = NewTimeValue;
        }

        m_PreviousTimeNs = NewTimeNs;
    }

    void keyboard(int Key, int Modifier)
    {
        switch(Key)
        {
            case 'w':
            case 'W':
            {
                HandleInput(&m_Buttons[PA_MoveForward]);
            } break;

            case 'a':
            case 'A':
            {
                HandleInput(&m_Buttons[PA_MoveLeft]);
            } break;

            case 's':
            case 'S':
            {
                HandleInput(&m_Buttons[PA_MoveBackward]);
            } break;

            case 'd':
            case 'D':
            {
                HandleInput(&m_Buttons[PA_MoveRight]);
            } break;

            default: {} break;
        }
    }

private:
    game_button *m_Buttons;
    qint64 m_PreviousTimeNs;
    QElapsedTimer m_Timer;
};

#endif // INPUT_H
