#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <iostream>

int main() {
    if ((SDL_Init(SDL_INIT_GAMECONTROLLER) == -1)) { 
        std::cout<<"Could not initialize SDL: "<<SDL_GetError()<<std::endl;
        return -1;
    }

    std::cout<<"SDL initialized"<<std::endl;

    if (SDL_NumJoysticks() < 1) {
        std::cout<<"No joysticks available"<<std::endl;
        SDL_Quit();
        return -1;
    }

    std::cout<<"Number of controllers: "<<SDL_NumJoysticks()<<std::endl;

    if (!SDL_IsGameController(0)) {
        std::cout<<"Joystick not supported"<<std::endl;
        return -1;
    }

    SDL_GameController* controller = SDL_GameControllerOpen(0);

    if (controller == nullptr) {
        std::cout<<"Error opening controller"<<std::endl;
        return -1;
    }

    std::cout<<"Using "<<SDL_GameControllerName(controller)<<std::endl;

    char guid[33];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(controller)), guid, sizeof(guid));
    std::cout << "Controller GUID: " << guid << std::endl;

    while (true) {
        int right_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int left_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);

        int right_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        int left_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        // for debuggin purposes
        if (!(right_x == 0 && left_x == 0 && right_y == 0 && left_y == 0)) {
            std::cout<<"x: ("<<right_x<<", "<<left_x<<")"<<std::endl;
            std::cout<<"y: ("<<right_y<<", "<<left_y<<")"<<std::endl;
        }

        if (SDL_GetError()) {
            std::cout<<SDL_GetError()<<std::endl;
            return -1;
        }
    }

    SDL_GameControllerClose(controller);
    SDL_Quit();

    return 0;
}