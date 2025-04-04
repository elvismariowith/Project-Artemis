#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <optional>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>


#include "environment_manager.h"
#include "serial_port.h"

// debugging
//#define SERIAL_PORT "/dev/ttyACM0"

SDL_TimerID axisTimerID = 0;
int currentDirection = 0; // -1 for left, 1 for right, 0 for neutral

Uint32 AxisTimerCallback(Uint32 interval, void* param) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = currentDirection;
    SDL_PushEvent(&event);


    return interval;
}

void arduinoCommunication(std::ofstream &arduino_serial, int command){
    arduino_serial << command << std::endl;
}

SDL_GameController* detectController() {
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

    SDL_GameController* controller = SDL_GameControllerOpen(0);
    if (!controller) {
        std::cerr << "The controller failed to open." << std::endl;
        return nullptr;
    }

    return controller;
}

int main() {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GameController* controller = detectController();
    if (!controller) {
        SDL_Quit();
        return -1;
    }


    SerialPort serialPort = [&]() -> SerialPort {
    for (int i = 0; i < 10; ++i) {
        std::string devPath = "/dev/ttyACM" + std::to_string(i);
        try {
            SerialPort sp(devPath);
            std::cout << "Connected to Arduino on: " << sp.getName() << std::endl;
            return sp;
        } catch (...) {
            // Fail silently, continue scanning
        }
    }

    std::cerr << "Failed to detect Arduino on any /dev/ttyACM* port." << std::endl;
    exit(-1);
    }();

    // maybe this isn't sending with the correct baud rate?
    std::ofstream arduino_serial(serialPort.getName()); // gets the file stream
        if(!arduino_serial.is_open()){
            std::cerr << "Failed to open arduino serial port." << std::endl;
        } 

    bool running = true;
    SDL_Event event;


    while (running) {
       

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

        else if (event.type == SDL_USEREVENT) {
            int command = event.user.code;
            arduinoCommunication(arduino_serial, command);

        }

        else if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
            int value = event.caxis.value;

            if (value < -8000) {
                if (currentDirection != 1) {
                    currentDirection = 1;
                    if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
                    axisTimerID = SDL_AddTimer(50, AxisTimerCallback, nullptr);
                    std::cout << "Moving left. Started timer ID: " << axisTimerID << std::endl;
                }
            }

            else if (value > 8000) {
                if (currentDirection != -1) {
                    currentDirection = -1;
                    if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
                    axisTimerID = SDL_AddTimer(50, AxisTimerCallback, nullptr);
                    std::cout << "Moving right. Started timer ID: " << axisTimerID << std::endl;
                }
            }

            else {
                if (currentDirection != 0) {
                    currentDirection = 0;
                    if (axisTimerID != 0) {
                        SDL_RemoveTimer(axisTimerID);
                        std::cout << "Stopped timer ID: " << axisTimerID << std::endl;
                        axisTimerID = 0;
                    }
                }
            }
        }

        else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
                std::cout << "X button pressed!\n" << std::endl;
            }
        }
    }

    SDL_Delay(1); // prevent tight loop
}

    if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}