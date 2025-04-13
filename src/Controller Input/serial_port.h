#include <optional>
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

enum SerialPortError {
    InvalidHandleValue,
    WriteError,
};

#ifdef _WIN32
HANDLE openSerialPort(std::string &serialPort);
#endif

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

std::optional<SerialPort> findArduinoSerialPort();