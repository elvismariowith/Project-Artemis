#ifndef ENVIRONMENT_MANAGER_H
#define ENVIRONMENT_MANAGER_H
#include <string>
#include <optional>

/// Manager to work with and load variables from the environment and load `.env` files.
class EnvironmentManager {
    public:
    /// Creates an empty `EnvironmentManager` to manage environment variables.
    EnvironmentManager() noexcept {  };

    /// Loads a `.env` file from the given `path`.
    /// If no path is given it defaults to searching for a `.env` file in the current directory.
    /// Returns `false` if it could not load the requested `.env` file.
    /// Returns `true` otherwise.
    bool loadEnvironment(const std::string &path = "./.env") const;

    /// Checks if the given variable exists in the environment.
    /// Return `true` if the variable exists, `false` otherwise.
    bool exists(const std::string &variable) const;

    /// Gets the value of the given `variable` from the environment.
    /// Returns `std::nullopt` if the variable doesn't exist. Returns `std::string` otherwise.
    std::optional<std::string> get(const std::string &variable) const;
};
#endif