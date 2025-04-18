#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <stdio.h>
#include <cstdio> // for popen and pclose
#include <string>

#include "voice.hpp"


#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif


int voiceControl(SerialPort& arduinoPort) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("python Voicerecognition.py", "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run Python script\n";
        return 1;
    }
    std::cout << "Python Output:\n";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
        //std::cout << buffer.data();
    }
    if(result == "" || result == "\n"){
        return -1;
    }
    int input = std::stoi(result);
    std::cout << "Vocal Output: " <<input;

    
    arduinoPort.write(input);
    if(input == 3)
        arduinoPort.write(-3);

    return 0;
}
