#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif
struct PCloseDeleter {
    void operator()(FILE* f) const {
        if (f) pclose(f);
    }
};
int voiceControl(); // Forward declaration needed

int main() {
    const char* portname = "/dev/ttyACM0"; 

    int serialPort = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serialPort < 0) {
        std::cerr << "Error opening " << portname << ": " << strerror(errno) << std::endl;
        return 1;
    }

    // Configure serial port
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(serialPort, &tty) != 0) {
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        close(serialPort);
        return 1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        close(serialPort);
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    int servoAngle = voiceControl();
    if (servoAngle == -1) {
        std::cerr << "No valid voice input. Exiting.\n";
        close(serialPort);
        return 1;
    }

    std::string angle = std::to_string(servoAngle) + "\n";
    int n_written = write(serialPort, angle.c_str(), angle.size());
    if (n_written < 0) {
        std::cerr << "Error writing to serial port\n";
    } else {
        std::cout << "Sent: " << angle;
    }

    close(serialPort);
    return 0;
}

// Function that runs voice recognition script and maps response to servo angle
int voiceControl() {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, PCloseDeleter> pipe(popen("/home/daniel/projects/voiceRecognition/vvv/bin/python ../Voicerecognition.py", "r"));
    if (!pipe) {
        std::cerr << "Failed to run Python script\n";
        return -1;
    }

    std::cout << "Python Output:\n";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if(result.empty() || result == "\n"){
        return -1;
    }

    try {
        int input = std::stoi(result);
        std::cout << "Vocal Output: " << input << "\n";
        if (input == 1) return 10;
        else if (input == -1) return 170;
        else std::cout << "Invalid input\n";
    } catch (std::exception& e) {
        std::cerr << "Conversion error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
