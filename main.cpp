#include "vision.hpp"
#include <iostream>
#include <string>
int main(int argc, char** argv){
    if(argc <= 1){
        std::cout<<"Not enough arguments\n";
        return 1;
    }

    bool modelSaved = (std::string(argv[1]) == "1");
    automatedMode(modelSaved);
}
