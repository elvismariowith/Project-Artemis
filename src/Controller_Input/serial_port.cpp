#include <vector>
#include <iostream>
#include "serial_port.hpp"

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

    cfsetospeed(&tty, B4800);
    cfsetispeed(&tty, B4800);

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

SerialPort::SerialPort(std::string name): arduino_serial(name) {
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

std::optional<SerialPortError> SerialPort::write(int command)  {
    
    arduino_serial << command << std::endl;
    return std::nullopt;
}

/*std::optional<SerialPort> findArduinoSerialPort() {
    #ifdef _WIN32
    HDEVINFO ports = SetupDiGetClassDevsW(&GUID_DEVCLASS_PORTS, L"USB", nullptr, DIGCF_PRESENT);

    SP_DEVINFO_DATA info;
    info.cbSize = sizeof(SP_DEVINFO_DATA);

    for (int i = 0; SetupDiEnumDeviceInfo(ports, i, &info); ++i) {
        unsigned char deviceName[256];

        bool success = SetupDiGetDeviceRegistryPropertyA(ports, &info, SPDRP_FRIENDLYNAME, nullptr, deviceName, sizeof(deviceName), nullptr);

        if (success) {
            if (std::string((char *)deviceName).substr(0, 7) == "Arduino") {
                HKEY hKey = SetupDiOpenDevRegKey(ports, &info, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                TCHAR portName[256];
                DWORD portNameSize = sizeof(portName);
                
                if (RegQueryValueExA(hKey, "PortName", NULL, NULL, (LPBYTE)portName, &portNameSize) != ERROR_SUCCESS) {
                    std::cerr << "Failed to get port name from registry." << std::endl;
                    RegCloseKey(hKey);
                    continue;
                }
                RegCloseKey(hKey);
        
                std::cout << "Found Arduino on: " << portName << std::endl;
        
                SetupDiDestroyDeviceInfoList(ports);
                return SerialPort(std::string((char *)portName));
            }
        }
    }

    SetupDiDestroyDeviceInfoList(ports);

    return std::nullopt;

} */

SerialPort findArduinoSerialPort() {
    // This function would require scanning /dev/tty* and possibly reading udev properties or using libudev
    // Placeholder for Linux implementation
 
    for (int i = 0; i < 10; ++i) {
        std::string devPath = "/dev/ttyACM" + std::to_string(i);
        SerialPort sp(devPath);
        std::cout << "Connected to Arduino on: " << sp.getName() << std::endl;
        return std::move(sp);        
    }

}

SerialPort::SerialPort(SerialPort&& other) noexcept {
    this->name = std::move(other.name);
    this->serialPort = std::move(other.serialPort);
}

SerialPort& SerialPort::operator=(SerialPort&& other) noexcept {
    if (this != &other) {
        this->name = std::move(other.name);
        this->serialPort = std::move(other.serialPort);
    }
    return *this;
}
