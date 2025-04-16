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
#include "serial_port.hpp"

// debugging
//#define SERIAL_PORT "/dev/ttyACM0"

SDL_TimerID axisTimerIDx = 0;
SDL_TimerID axisTimerIDy = 0;
SDL_TimerID axisTimerTrigger = 0;

int currentInputX = 0; // -1 for left, 1 for right, 0 for neutral
int currentInputY = 0;
int currentInputTrigger = -3; // -3 for no trigger pressed

Uint32 AxisTimerCallbackX(Uint32 interval, void* param) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = currentInputX;
    SDL_PushEvent(&event);


    return interval;
}

Uint32 AxisTimerCallbackY(Uint32 interval, void* param) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = currentInputY;
    SDL_PushEvent(&event);


    return interval;
}

Uint32 AxisTimerCallbackTrigger(Uint32 interval, void* param) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = currentInputTrigger;
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
        
        // Horizontal movement
        else if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
            int value = event.caxis.value;

            if (value < -8000) {
                if (currentInputX != 1) { // if new input detected..
                    currentInputX = 1; // send data to move left
                    if (axisTimerIDx != 0) SDL_RemoveTimer(axisTimerIDx); // stop existing timers from other previous inputs 
                    axisTimerIDx = SDL_AddTimer(50, AxisTimerCallbackX, nullptr); // start event!
                     std::cout << "Moving left. Started timer ID: " << axisTimerIDx << std::endl;
                }
            }


            else if (value > 8000) {
                if (currentInputX != -1) {
                    currentInputX = -1;
                    if (axisTimerIDx != 0) SDL_RemoveTimer(axisTimerIDx);
                    axisTimerIDx = SDL_AddTimer(50, AxisTimerCallbackX, nullptr);
                     std::cout << "Moving right. Started timer ID: " << axisTimerIDx << std::endl;
                }
            }

            else {
                if (currentInputX != 0) {
                    currentInputX = 0;
                    if (axisTimerIDx != 0) {
                        SDL_RemoveTimer(axisTimerIDx);
                        std::cout << "Stopped timer ID: " << axisTimerIDx << std::endl;
                        axisTimerIDx = 0;
                    }
                }
            }
        }

        // Vertical movement
        else if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY){
            int value = event.caxis.value;

            if (value < -8000){
                if (currentInputY != 2) {
                    currentInputY = 2;
                    if (axisTimerIDy != 0) SDL_RemoveTimer(axisTimerIDy);
                    axisTimerIDy = SDL_AddTimer(50, AxisTimerCallbackY, nullptr);
                    std::cout << "Moving up. Started timer ID: " << axisTimerIDy << std::endl;
                }
            } else if (value > 8000) {
                if (currentInputY != -2) {
                    currentInputY = -2;
                    if (axisTimerIDy != 0) SDL_RemoveTimer(axisTimerIDy);
                    axisTimerIDy = SDL_AddTimer(50, AxisTimerCallbackY, nullptr);
                    std::cout << "Moving down. Started timer ID: " << axisTimerIDy << std::endl;
                }
            } else { // Axis in neutral position.
                currentInputY = 0;
                if (axisTimerIDy != 0) {
                    SDL_RemoveTimer(axisTimerIDy);
                    axisTimerIDy = 0;
                }
            }
        } 
    
        // Trigger input
        else if (event.type == SDL_CONTROLLERAXISMOTION) {
            if (event.cbutton.button == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                int value = event.caxis.value;
                
                if (value > 8000){
                if (currentInputTrigger != 3){
                    std::cout <<"Right trigger pressed (value: )" << value << ")\n";
                    currentInputTrigger = 3;
                    if (axisTimerTrigger != 0){
                        SDL_RemoveTimer(axisTimerTrigger);
                        axisTimerTrigger = 0;
                    } 
                    axisTimerTrigger = SDL_AddTimer(50, AxisTimerCallbackTrigger, nullptr);
                }
                else if (value < 8000){
                    if (currentInputTrigger != -3){
                    std::cout <<"Right trigger released (value: )" << value << ")\n";
                    currentInputTrigger = -3;
                    if (axisTimerTrigger != 0) {
                        SDL_RemoveTimer(axisTimerTrigger);
                        axisTimerTrigger = 0;
                    }
                    axisTimerTrigger = SDL_AddTimer(50, AxisTimerCallbackTrigger, nullptr);
                }
                //std::cout << "Shooting...\n" << std::endl;
            }
            }
            }
        } 
    }
    SDL_Delay(1); // prevent tight loop
}

    if (axisTimerIDy != 0) SDL_RemoveTimer(axisTimerIDy);
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}