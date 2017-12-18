#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H

#include "controllablecamera.h"

class DebugCamera : public ControllableCamera
{
public:
    DebugCamera()
        : ControllableCamera(),
          ProcessingInput(false)
    {

    }

    void doIt() override
    {
        if(ProcessingInput)
        {
            ControllableCamera::doIt();
        }
    }

    bool ProcessingInput;
};

#endif // DEBUG_CAMERA_H
