#include <environment_manager.h>
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

    if (file.is_open()) {
        std::string line;

        while (std::getline(file, line)) {
            size_t delimiterPos = line.find("=");

            if (delimiterPos != std::string::npos) {
                std::string var = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1, delimiterPos);
                
                // Get rid of the opening double quote
                if (value[0] == '"') {
                    value = value.substr(1);
                }

                // Get rid of closing double quote
                size_t index = value.find_first_of('"');
                if (index) {
                    value = value.substr(0, index);
                }

                #ifdef _WIN32
                putenv((var + "=" + value).c_str()); // Overwrite if exists
                #endif
            }
        }

        file.close();
    } else {
        std::cerr << "Error opening .env file: " << filePath << std::endl;
        return;
    }
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

    if (value == nullptr) {
        return std::nullopt;
    }

    return std::string(value);
}