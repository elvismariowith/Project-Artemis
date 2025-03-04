#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <devguid.h>
#endif

#include "environment_manager.h"
#include "serial_port.h"

// Global variables for managing repeated commands.
SDL_TimerID axisTimerID = 0;
int currentDirection = 0; // -1 for left, 1 for right, 0 for neutral

// This callback is invoked on a fixed interval when the axis is activated.
Uint32 AxisTimerCallback(Uint32 interval, void* param) {
    // Post a custom event with the current direction as the event code.
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = currentDirection;
    SDL_PushEvent(&event);
    return interval; // Reschedule the timer to run repeatedly.
}

SDL_GameController* detectController(){
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
    EnvironmentManager envManager;
    envManager.loadEnvironment("./.env");

    std::optional<std::string> serialPortName = envManager.get("ARDUINO_SERIAL_PORT");

    SerialPort serialPort = [&]() -> SerialPort {
        if (serialPortName.has_value()) {
            serialPort = SerialPort(serialPortName.value());
            std::cout << "Arduino's serial port set to " << serialPort.getName() << std::endl;

            return serialPort;
        } else {
            std::cout << "No ARDUINO_SERIAL_PORT set. Defaulting to automatic finding." << std::endl;

            std::optional<SerialPort> result = findArduinoSerialPort();
            if (result.has_value()) {
                serialPort = result.value();
                std::cout << "Found " << serialPort.getName() << ". Defaulting to serial port " << serialPort.getName() << std::endl;

                return serialPort;
            } else {
                std::cerr << "No Arduino serial port found.";
                exit(-1);
            }
        }
    }();
    
    // Initialize SDL subsystems.
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    SDL_GameController* controller = detectController();
    if (!controller) {
        SDL_Quit();
        return -1;
    }
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        // Process all pending events.
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // Custom event posted by the timer.
            else if (event.type == SDL_USEREVENT) {
                int command = event.user.code;
                std::optional<SerialPortError> result = serialPort.write(std::to_string(command));

                if (result.has_value()) {
                    std::cerr << "Failed to write to serial port. Error code: " << GetLastError() << std::endl;
                }
            }
            // Process controller axis motions.
            else if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                int value = event.caxis.value;
                if (value < -8000) {
                    if (currentDirection != 1) { // Only change state when needed.
                        currentDirection = 1;
                        if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
                        axisTimerID = SDL_AddTimer(50, AxisTimerCallback, nullptr); // Repeat every 50ms.
                    }
                } else if (value > 8000) {
                    if (currentDirection != -1) {
                        currentDirection = -1;
                        if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
                        axisTimerID = SDL_AddTimer(50, AxisTimerCallback, nullptr);
                    }
                } else { // Axis in neutral position.
                    currentDirection = 0;
                    if (axisTimerID != 0) {
                        SDL_RemoveTimer(axisTimerID);
                        axisTimerID = 0;
                    }
                }
            }
        }
        SDL_Delay(1); // A short delay to prevent a tight busy-loop.
    }
    
    if (axisTimerID != 0) SDL_RemoveTimer(axisTimerID);
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
