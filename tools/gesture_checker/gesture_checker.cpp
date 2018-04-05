#include "stdafx.h"
#include "CGestureListener.h"

int main(int argc, char** argv)
{
    CGestureListener listener;
    Leap::Controller controller;

    controller.addListener(listener);
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    controller.setPolicy(Leap::Controller::POLICY_ALLOW_PAUSE_RESUME);

    std::cout << "Press Enter to quit, or enter 'p' to pause or unpause the service..." << std::endl;

    bool paused = false;
    while(true)
    {
        char c = std::cin.get();
        if(c == 'p')
        {
            paused = !paused;
            controller.setPaused(paused);
            std::cin.get(); //skip the newline
        }
        else break;
    }

    controller.removeListener(listener);

    return EXIT_SUCCESS;
}
