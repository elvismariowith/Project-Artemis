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

// Direction enum
// 1's for horizontal
// 2's for vertical
// 3's for trigger
enum Direction {
    NONE = 0,
    LEFT = 1,
    RIGHT = -1,
    UP = 2,
    DOWN = -2
};

// Timer context struct
struct TimerContext {
    Direction dir;
};

// Timer contexts
TimerContext leftCtx = { LEFT };
TimerContext rightCtx = { RIGHT };
TimerContext upCtx = { UP };
TimerContext downCtx = { DOWN };

// Timer IDs and current state
SDL_TimerID axisTimerID = 0;
SDL_TimerID yAxisTimerID = 0;

int currentDirectionX = NONE;
int currentDirectionY = NONE;

// Timer callback function
Uint32 AxisTimerCallback(Uint32 interval, void* param) {
    TimerContext* ctx = static_cast<TimerContext*>(param);
    
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = ctx->dir;  // Send -2, -1, 0, 1, or 2
    SDL_PushEvent(&event);

    return interval;
}

// Send command to Arduino
void arduinoCommunication(std::ofstream &arduino_serial, int command) {
    arduino_serial << command << std::endl;
}

// Detect controller
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
                // Fail silently
            }
        }

        std::cerr << "Failed to detect Arduino on any /dev/ttyACM* port." << std::endl;
        exit(-1);
    }();

    std::ofstream arduino_serial(serialPort.getName());
    if (!arduino_serial.is_open()) {
        std::cerr << "Failed to open arduino serial port." << std::endl;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // Event sent from timer callback
            else if (event.type == SDL_USEREVENT) {
                int command = event.user.code;
                arduinoCommunication(arduino_serial, command);
                std::cout << "Sent to Arduino: " << command << std::endl;
            }

            // Left stick X-axis (left/right)
            else if (event.type == SDL_CONTROLLERAXISMOTION &&
                     event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {

                int value = event.caxis.value;
                Direction newDirectionX = NONE;

                if (value < -8000) newDirectionX = LEFT;
                else if (value > 8000) newDirectionX = RIGHT;

                if (newDirectionX != currentDirectionX) {
                    currentDirectionX = newDirectionX;

                    if (axisTimerID != 0) {
                        SDL_RemoveTimer(axisTimerID);
                        std::cout << "Stopped X-axis timer ID: " << axisTimerID << std::endl;
                        axisTimerID = 0;
                    }

                    if (currentDirectionX != NONE) {
                        TimerContext* ctx = (currentDirectionX == LEFT) ? &leftCtx : &rightCtx;
                        axisTimerID = SDL_AddTimer(50, AxisTimerCallback, ctx);
                        std::cout << "Started X-axis timer for direction: " << currentDirectionX
                                  << " | Timer ID: " << axisTimerID << std::endl;
                    } else {
                        std::cout << "X-axis stick returned to neutral.\n";
                    }
                }
            }

            // Left stick Y-axis (up/down)
            else if (event.type == SDL_CONTROLLERAXISMOTION &&
                     event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {

                int value = event.caxis.value;
                Direction newDirectionY = NONE;

                if (value < -8000) newDirectionY = UP;
                else if (value > 8000) newDirectionY = DOWN;

                if (newDirectionY != currentDirectionY) {
                    currentDirectionY = newDirectionY;

                    if (yAxisTimerID != 0) {
                        SDL_RemoveTimer(yAxisTimerID);
                        std::cout << "Stopped Y-axis timer ID: " << yAxisTimerID << std::endl;
                        yAxisTimerID = 0;
                    }

                    if (currentDirectionY != NONE) {
                        TimerContext* ctx = (currentDirectionY == UP) ? &upCtx : &downCtx;
                        yAxisTimerID = SDL_AddTimer(50, AxisTimerCallback, ctx);
                        std::cout << "Started Y-axis timer for direction: " << currentDirectionY
                                  << " | Timer ID: " << yAxisTimerID << std::endl;
                    } else {
                        std::cout << "Y-axis stick returned to neutral.\n";
                    }
                }
            }

            // Button press debug
            else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
                    std::cout << "X button pressed!\n";
                }
            }
        }

        SDL_Delay(1);
    }

    // Cleanup
    if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
    if (yAxisTimerID != 0) SDL_RemoveTimer(yAxisTimerID);
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
