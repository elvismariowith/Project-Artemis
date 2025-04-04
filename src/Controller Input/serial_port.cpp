#include "serial_port.h"
// FILE CONTROLS THE SETTINGS AND THE PARAMETERS OF THE SELECTED SERIAL PORT
#include <string>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <optional>

SerialPort::SerialPort(std::string name) : name(name) {
    fd = open(name.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    std::cout << "FD is currently : " << fd << std::endl;
    if (fd < 0) {
        std::cerr << "Failed to open serial port " << name << ": " << strerror(errno) << std::endl;
        throw SerialPortError::InvalidHandleValue;
    }

    termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        throw SerialPortError::InvalidHandleValue;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 1;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        throw SerialPortError::InvalidHandleValue;
    }
}

SerialPort::~SerialPort() {
    if (fd < 0) {
        close(fd);
    }
}

std::optional<SerialPortError> SerialPort::write(int command) {
    if (fd < 0) {
        std::cerr << "Invalid file descriptor." << std::endl;
        return SerialPortError::WriteError;
    }

    std::string message = std::to_string(command) + "\n"; // convert into a stream of text
    ssize_t result = ::write(fd, message.c_str(), message.size());

    if (result < 0) {
        std::cerr << "Write failed: " << strerror(errno) << std::endl;
        return SerialPortError::WriteError;
    }

    tcflush(fd, TCIOFLUSH); 
    return std::nullopt;
}

std::optional<SerialPort> findArduinoSerialPort() {
    for (int i = 0; i < 10; ++i) {
        std::string devPath = "/dev/ttyACM" + std::to_string(i);
        std::cout << "Trying " << devPath << "..." << std::endl;

        try {
            return SerialPort(devPath);
        } catch (...) {
            std::cerr << "Failed to open " << devPath << std::endl;
        }
    }

    std::cerr << "No accessible /dev/ttyACM* ports found." << std::endl;
    return std::nullopt;
    #endif
}
