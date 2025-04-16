#include <string>
#include <iostream>
#include <memory>
#include <optional>
#include "serial_port.h"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>

int openSerialPort(const std::string &serialPort) {
    int fd = open(serialPort.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "Error opening " << serialPort << ": " << strerror(errno) << std::endl;
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "Error getting tty attributes: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);               // ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);             // no parity
    tty.c_cflag &= ~CSTOPB;                        // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                       // no hardware flow control

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error setting tty attributes: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    return fd;
}

SerialPort::SerialPort(std::string name) {
    int fd = openSerialPort(name);
    if (fd < 0) {
        throw SerialPortError::InvalidHandleValue;
    }

    this->name = name;
    this->serialPort = std::shared_ptr<void>((void *)(intptr_t)fd, [](void* ptr) {
        int fd = (intptr_t)ptr;
        if (fd >= 0) {
            std::cout << "closing fd" << std::endl;
            close(fd);
        }
    });
}

std::optional<SerialPortError> SerialPort::write(std::string message) const {
    if (message.back() != '\n') {
        message += '\n';
    }

    int fd = (intptr_t)this->serialPort.get();
    ssize_t written = ::write(fd, message.c_str(), message.size());
    if (written < 0) {
        return SerialPortError::WriteError;
    }

    // Flush both input and output buffers
    tcflush(fd, TCIOFLUSH);
    return std::nullopt;
}

std::optional<SerialPort> findArduinoSerialPort() {
    // This function would require scanning /dev/tty* and possibly reading udev properties or using libudev
    // Placeholder for Linux implementation
    return std::nullopt;
}

#endif
