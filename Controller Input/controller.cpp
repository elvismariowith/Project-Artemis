#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

#define SERIAL_PORT "/dev/ttyACM0"

SDL_GameController* controller = NULL;

void arduinoCommunication(int angle){
    std::ofstream serial(SERIAL_PORT);
    if (serial.is_open()){
        serial << angle << std::endl;
        serial.close();
        std::cout << "Sent to Arduino successfully" << angle << std::endl;
    } else{
        std::cerr << "Failed to open serial port" << std::endl;
    }
}

SDL_GameController* detectController(){

    int numJoysticks = SDL_NumJoysticks();
    std::cout << "Number of connected controllers: " << numJoysticks << std::endl;

    if (numJoysticks > 0) {
            if (SDL_IsGameController(0)) {
                controller = SDL_GameControllerOpen(0);
            }
            else {
                std::cout << "No controllers detected." << std::endl;
            }
    }
    return controller;
}

void readControllerInput(){

    bool running = true;
    SDL_Event event;
    while(running) {
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
            }
            if (event.type == SDL_CONTROLLERBUTTONDOWN){
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A){
                    std::cout << "X was pressed!" << std::endl;
                }
            }

            if (event.type == SDL_CONTROLLERAXISMOTION) {
                if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    int value = event.caxis.value;

                    if (value < -8000){
                        std::cout << "Moving Left! Value: " << value << std::endl;
                        int angle = (value + 32768) * 180 / 65535;
                        arduinoCommunication(angle);
                        usleep(50000);

                    } else if (value > 8000){
                        std::cout << "Moving Right! Value: " << value << std::endl;
                        int angle = (value + 32768) * 180 / 65535;
                        arduinoCommunication(angle);
                        usleep(50000);
                    }
                }
            }
        }
    }

}


int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    } else{
        std::cout << "SDL initialized!" <<std::endl;
    }

    // Controller setup here

    controller = detectController();

    // Detecting controller input here...

    readControllerInput();
    
    // Ya gotta clean up after yourself
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
