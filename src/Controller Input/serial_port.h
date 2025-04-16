#include <optional>
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

enum SerialPortError {
    InvalidHandleValue,
    WriteError,
};

#ifdef _WIN32
HANDLE openSerialPort(std::string &serialPort);
#else
int openSerialPort(const std::string &serialPort);
#endif

/// Wrapper over os-dependent serial port
class SerialPort {
    std::string name;
    std::shared_ptr<void> serialPort;

    public:
    /// Accepts port names such as `/dev/ttyUSB0` on Linux or `COM7` on Windows
    SerialPort(std::string name);

    /// Writes the given `message` to the serial port, appending a `\n` character at the end
    /// to signal the end of the message being written.
    /// Throws `SerialPortError::WriteError` in the case of an error.
    std::optional<SerialPortError> write(std::string message) const;

    /// Returns the name of the serial port
    const std::string& getName() const noexcept {
        return this->name;
    }
};

std::optional<SerialPort> findArduinoSerialPort();
