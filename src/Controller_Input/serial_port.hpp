#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <optional>
#include <string>
#include <memory>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>


//#ifndef _WIN32
//#include <unistd.h>
//#endif

enum SerialPortError {
    InvalidHandleValue,
    WriteError,
};

//HANDLE openSerialPort(std::string &serialPort);
//#else
int openSerialPort(const std::string &serialPort);

//=======
//>>>>>>> origin/Windows:src/Controller_Input/serial_port.hpp

/// Wrapper over os-dependent serial port
class SerialPort {
    std::string name;
    std::shared_ptr<void> serialPort;
    std::ofstream arduino_serial;
    public:
    /// Accepts port names such as `/dev/ttyUSB0` on Linux or `COM7` on Windows
    SerialPort(std::string name);

    /// Writes the given `message` to the serial port, appending a `\n` character at the end
    /// to signal the end of the message being written.
    /// Throws `SerialPortError::WriteError` in the case of an error.
    std::optional<SerialPortError> write(int command);

    /// Returns the name of the serial port
    const std::string& getName() const noexcept {
        return this->name;
    }

//<<<<<<< HEAD:src/Controller Input/serial_port.h
//=======
    // Disable copying
    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    // Enable moving
    SerialPort(SerialPort&& other) noexcept;
    SerialPort& operator=(SerialPort&& other) noexcept;
};
SerialPort loadServo();
SerialPort findArduinoSerialPort();
void writeCommandsThread(SerialPort& serialPort,int command,int repetitions,int milliseconds);
#endif
//>>>>>>> origin/Windows:src/Controller_Input/serial_port.hpp
