#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include "json.hpp"// Include nlohmann/json.hpp or install via package manager


using json = nlohmann::json;

int main() {
    std::string command = "python speech_recognizer.py";
    int status = system(command.c_str());

    if (status != 0) {
        std::cerr << "Failed to run Python script." << std::endl;
        return 1;
    }

    std::ifstream inputFile("speech_output.json");
    if (!inputFile) {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    json output;
    inputFile >> output;

    if (output.contains("error")) {
        std::cerr << "Speech recognition error: " << output["error"] << std::endl;
        return 1;
    }

    std::cout << "You said: " << output["text"] << std::endl;
    std::cout << "Detected keywords:" << std::endl;
    for (const auto& word : output["keywords"]) {
        std::cout << word << std::endl;
    }

    return 0;
}
