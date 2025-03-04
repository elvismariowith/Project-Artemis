#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <iostream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <devguid.h>
#endif
#include "environment_manager.h"

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

#ifdef _WIN32
HANDLE openSerialPort(std::string &serialPort) {
    HANDLE hSerial = CreateFileA(serialPort.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening " << serialPort << ". Error code: " << GetLastError() << std::endl;
        return INVALID_HANDLE_VALUE;
    }
    
    // Set serial port parameters.
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state." << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state." << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    
    // Set timeouts for read/write operations.
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting timeouts." << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    
    return hSerial;
}
#endif

#ifdef _WIN32
void arduinoCommunication(HANDLE serialPort, int command) {
    std::string msg = std::to_string(command) + "\n";
    DWORD bytesWritten;
    if (!WriteFile(serialPort, msg.c_str(), (DWORD)msg.size(), &bytesWritten, nullptr)) {
        std::cerr << "Failed to write to serial port. Error code: " << GetLastError() << std::endl;
    }  

    // clear input and output buffer
    PurgeComm(serialPort, 0b1100);
}
#endif

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

    bool serialPortSet = false;
    std::string serialPortName;

    if (envManager.exists("ARDUINO_SERIAL_PORT")) {
        // Doesn't check for std::nullopt due to previous .exists() call which prevents UB
        serialPortName = envManager.get("ARDUINO_SERIAL_PORT").value();
        
        #ifdef _WIN32
        if (serialPortName.substr(0, 4) != "\\\\.\\") {
            serialPortName = "\\\\.\\" + serialPortName;
        }
        #endif
        
        std::cout << "Arduino's serial port set to " << serialPortName << std::endl;
        serialPortSet = true;
    } else {
        std::cout << "No ARDUINO_SERIAL_PORT set. Defaulting to automatic finding." << std::endl;
    }
    
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

    if (!serialPortSet) {
        #ifdef _WIN32
        HDEVINFO ports = SetupDiGetClassDevsW(&GUID_DEVCLASS_PORTS, L"USB", nullptr, DIGCF_PRESENT);
    
        SP_DEVINFO_DATA info;
        info.cbSize = sizeof(SP_DEVINFO_DATA);
    
        for (int i = 0; SetupDiEnumDeviceInfo(ports, i, &info); ++i) {
            unsigned char deviceName[256];
            SetupDiGetDeviceRegistryPropertyA(ports, &info, SPDRP_FRIENDLYNAME, nullptr, deviceName, sizeof(deviceName), nullptr);
    
            HKEY hKey = SetupDiOpenDevRegKey(ports, &info, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            TCHAR portName[256];
            DWORD portNameSize = sizeof(portName);
            RegQueryValueExA(hKey, "PortName", NULL, NULL, (LPBYTE)portName, &portNameSize);
            RegCloseKey(hKey);
    
            if (deviceName != nullptr) {
                if (std::string((char *)deviceName).substr(0, 7) == "Arduino") {
                    serialPortName = std::string((char *)portName);

                    if (serialPortName.substr(0, 5) != "\\\\.\\") {
                        serialPortName = "\\\\.\\" + serialPortName;
                    }
    
                    std::cout << "Found " << deviceName << ". Defaulting to serial port " << serialPortName << std::endl;
                    serialPortSet = true;
                    break;
                }
            }
        }
    
        SetupDiDestroyDeviceInfoList(&info);
        #endif
    }

    #ifdef _WIN32
    HANDLE serialPort = openSerialPort(serialPortName);
    if (serialPort == INVALID_HANDLE_VALUE) {
        SDL_GameControllerClose(controller);
        SDL_Quit();
        return -1;
    }
    #endif
    
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
                arduinoCommunication(serialPort, command);
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
    #ifdef _WIN32
    CloseHandle(serialPort);
    #endif
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
