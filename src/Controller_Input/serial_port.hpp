#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <optional>
#include <string>
#include <memory>

#include <windows.h>

enum SerialPortError {
    InvalidHandleValue,
    WriteError,
};

HANDLE openSerialPort(std::string &serialPort);

/// Wrapper over os-dependent serial port
class SerialPort {
    std::string name;
    std::shared_ptr<void> serialPort;

    public:
    /// WINDOWS: Accepts port names such as `COM7` and `\\.\COM7`
    SerialPort(std::string name);

    /// Writes the given `message` to the serial port, appending a `\n` character at the end
    /// to signal the end of the message being written.
    /// Throws `SerialPortError::WriteError` in the case of an error.
    std::optional<SerialPortError> write(std::string message) const;

    /// Returns the name of the serial port
    const std::string& getName() const noexcept {
        return this->name;
    }

    // Disable copying
    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    // Enable moving
    SerialPort(SerialPort&& other) noexcept;
    SerialPort& operator=(SerialPort&& other) noexcept;
};
SerialPort loadServo();
std::optional<SerialPort> findArduinoSerialPort();

#endif