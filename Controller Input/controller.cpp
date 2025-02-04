#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <iostream>
#include <fstream>

#define SERIAL_PORT "/dev/ttyACM0"

SDL_GameController* detectController(){
    // Check if controller exists and is supported
    int numJoysticks = SDL_NumJoysticks();
    std::cout << "Number of connected controllers: " << numJoysticks << std::endl;

    if (numJoysticks <= 0) {
        std::cerr << "No controllers detected." << std::endl;
        return nullptr;
    }

    if (!SDL_IsGameController(0)) {
        std::cerr << "The controller is not supported." << std::endl;
        return nullptr;
    }

    // Open controller          
    SDL_GameController* controller = SDL_GameControllerOpen(0);

    if (!controller) {
        std::cerr << "The controller failed to open." << std::endl;
        return nullptr;
    }
    
    return controller;
}

void readControllerInput(std::ofstream &arduino_serial){
    bool running = true;
    SDL_Event event;
    
    while(running) {
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
            }
            else if (event.type == SDL_CONTROLLERBUTTONDOWN){
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A){
                    std::cout << "X was pressed!" << std::endl;
                }
            }

            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                std::cout << "Moving Left!" << std::endl;
                arduino_serial << -1;
            }
            else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                std::cout << "Moving Right!" << std::endl;
                arduino_serial << 1;
            }
        }
    }

}

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    std::cout << "SDL initialized!" << std::endl;

    // Controller setup here
    SDL_GameController* controller = detectController();

    std::ofstream arduino_serial(SERIAL_PORT);
    if (!arduino_serial.is_open()){
        std::cerr << "Failed to open arduino serial port." << std::endl;
    } 
    else {
        // Detecting controller input here...
        readControllerInput(arduino_serial);
    }

    // Ya gotta clean up after yourself
    arduino_serial.close();
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
