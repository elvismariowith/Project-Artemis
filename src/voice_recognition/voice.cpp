#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <stdio.h>
#include <cstdio> // for popen and pclose
#include <string>
#include <fstream>
#include "voice.hpp"


#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif


const std::string PATH = "../src/voice_recognition/";
int voiceControl(SerialPort& arduinoPort) {
    std::array<char, 128> buffer;
    std::string result = "";

    std::string python_file_path = "";
    python_file_path += "python " + PATH + "Voicerecognition.py";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(python_file_path.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run Python script\n";
        return 1;
    }
    std::ifstream file("voice_output.txt");
    if(file.is_open()){
        file >> result;
    }
    if(result == "" || result == "\n"){
        return -1;
    }
    int input = std::stoi(result);
    //std::cout << "Vocal Output: " <<input;

    for(int i = 0;i < 5;i++)
        arduinoPort.write(input);
    if(input == 3)
        arduinoPort.write(-3);

    return 0;
}
