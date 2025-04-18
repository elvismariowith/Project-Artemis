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
    // std::thread automated_thread(automatedMode, modelSaved);
    std::thread manual_thread(manualMode);


    manual_thread.join();
    //automated_thread.join();
}
