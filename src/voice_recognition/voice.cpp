#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <stdio.h>
#include <cstdio> // for popen and pclose

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif


int main() {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("python speech_recognizer.py", "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run Python script\n";
        return 1;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::cout << "Python Output:\n" << result;
    return 0;
}
