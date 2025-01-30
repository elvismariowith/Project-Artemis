#include <SDL2/SDL_gamecontroller.h>
#include <iostream>

int main() {
    int joystick_index = SDL_NumJoysticks();

    if (joystick_index < 1) {
        std::cout<<"No joysticks available"<<std::endl;
        return -1;
    }

    if (!SDL_IsGameController(joystick_index)) {
    std::cout<<"Joystick not supported"<<std::endl;
    return -1;
    }

    SDL_GameController* controller = SDL_GameControllerOpen(joystick_index);

    if (controller == nullptr) {
        std::cout<<"Error opening controller"<<std::endl;
        return -1;
    }

    SDL_GameControllerAxis axis = SDL_CONTROLLER_AXIS_LEFTX;

    while (true) {
        int right_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int left_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int right_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        int left_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    }

    return 0;
}