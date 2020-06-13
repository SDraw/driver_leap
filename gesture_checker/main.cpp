#include "stdafx.h"
#include "CGestureListener.h"

int main(int argc, char *argv[])
{
    CGestureListener l_listener;
    Leap::Controller l_controller;

    l_controller.addListener(l_listener);
    l_controller.setPolicy(Leap::Controller::POLICY_ALLOW_PAUSE_RESUME);

    std::cout << "Press Enter to quit, or enter 'p' to pause or unpause the service..." << std::endl;

    bool l_paused = false;
    while(true)
    {
        char l_char = std::cin.get();
        if(l_char == 'p')
        {
            l_paused = !l_paused;
            l_controller.setPaused(l_paused);
            std::cin.get(); //skip the newline
        }
        else break;
    }

    l_controller.removeListener(l_listener);

    return EXIT_SUCCESS;
}
