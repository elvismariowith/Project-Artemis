#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <optional>
#include <string>
#include <memory>



enum SerialPortError {
    InvalidHandleValue,
    WriteError,
};

/// Wrapper over os-dependent serial port
class SerialPort {
    std::string name;
    int fd = -1;

    public:
    SerialPort(std::string name);
    ~SerialPort();
    /// Writes the given `message` to the serial port, appending a `\n` character at the end
    /// to signal the end of the message being written.
    /// Throws `SerialPortError::WriteError` in the case of an error.
    std::optional<SerialPortError> write(int command);

    /// Returns the name of the serial port
    const std::string& getName() const noexcept {
        return this->name;
    }
};

std::optional<SerialPort> findArduinoSerialPort();

#endif