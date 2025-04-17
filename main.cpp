#include "vision.hpp"
#include "controller_linux.hpp"
#include "serial_port.hpp"
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv){
    if(argc <= 1){
        std::cout<<"Not enough arguments\n";
        return 1;
    }

    bool modelSaved = (std::string(argv[1]) == "1");
    SerialPort arduinoPort = findArduinoSerialPort();
    std::thread manual_thread(manualMode);
    std::thread automated_thread(automatedMode, modelSaved);

    //manual_thread.detach();
    automated_thread.join();

    //calls pytho voice recognition
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("python Voicerecognition.py", "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run Python script\n";
        return 1;
    }
    std::cout << "Python Output:\n";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result = buffer.data();
        //std::cout << buffer.data();
    }
    int input = stoi(result);
    std::cout << input;
}
