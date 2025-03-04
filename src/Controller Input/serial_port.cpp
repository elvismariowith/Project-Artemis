#include <string>
#include <iostream>
#include "serial_port.h"

#ifdef _WIN32
#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#endif

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

// TODO: Fix copy/move destroying serialPort pointer
SerialPort::SerialPort(std::string name) {
    #ifdef _WIN32
    // Serial ports in windows start with "\\.\" so we add it if the name doesn't have it
    if (name.substr(0, 4) != "\\\\.\\") {
        name = "\\\\.\\" + name;
    }

    HANDLE serialPort = openSerialPort(name);
    if (serialPort == INVALID_HANDLE_VALUE) {
        throw SerialPortError::InvalidHandleValue;
    }

    this->name = name;
    this->serialPort = std::shared_ptr<void>(serialPort,
        [](void* h) {
            if (h && h != INVALID_HANDLE_VALUE) {
                std::cout << "closing handle" << std::endl;
                CloseHandle(h);
            }
        }
    );
    #endif
}

std::optional<SerialPortError> SerialPort::write(std::string message) const {
    // Append `\n` character so that the Arduino knows it should read the message
    if (message[message.length() - 1] != '\n') {
        message += "\n";
    }

    if (!WriteFile(this->serialPort.get(), message.c_str(), (DWORD)message.size(), nullptr, nullptr)) {
        throw SerialPortError::WriteError;
    }

    // clear input and output buffer
    PurgeComm(this->serialPort.get(), 0b1100);
    return std::nullopt;
}

std::optional<SerialPort> findArduinoSerialPort() {
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
                SetupDiDestroyDeviceInfoList(&info);
                return SerialPort(std::string((char *)portName));
            }
        }
    }

    SetupDiDestroyDeviceInfoList(&info);

    return std::nullopt;
    #endif
}