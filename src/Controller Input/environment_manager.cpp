#include "environment_manager.h"
// FILE USED TO DYANMICALLY FIND THE SERIAL PORT THAT THE ARDUINO IS CONNECTED TO 
// USES WINDOWS API CURRENTLY
#include <string>
#include <optional>
#include <filesystem>
#include <iostream>
#include <fstream>

bool is_valid_filepath(const std::string& filePath) {
    std::filesystem::path path(filePath);
    return std::filesystem::exists(path);
}

void loadEnv(const std::string& filePath) {
    if (!is_valid_filepath(filePath)) {
        std::cerr << "File path is invalid." << std::endl;
        return;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening .env file: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments or blank lines
        if (line.empty() || line[0] == '#') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;

        std::string var = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Trim surrounding quotes if present
        if (!value.empty() && value.front() == '"') value.erase(0, 1);
        if (!value.empty() && value.back() == '"') value.pop_back();

        // Actually set the environment variable
        setenv(var.c_str(), value.c_str(), 1); // overwrite = 1
    }

    file.close();
}

bool EnvironmentManager::loadEnvironment(const std::string &path) const {
    if (!is_valid_filepath(path)) {
        return false;
    }

    loadEnv(path);
    return true;
}

bool EnvironmentManager::exists(const std::string &variable) const {
    return std::getenv(variable.c_str()) != nullptr;
}

std::optional<std::string> EnvironmentManager::get(const std::string &variable) const {
    const char *value = std::getenv(variable.c_str());
    if (value == nullptr) return std::nullopt;
    return std::string(value);
}