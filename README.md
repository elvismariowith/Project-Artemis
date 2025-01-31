<h1 align="center">
Project Artemis
</h1>

## Build
### Build controller input program
Install SDL2 dependencies needed for the input to work
```bash
# Installs SDL2 and its development libraries
sudo apt-get install libsdl2-2.0-0
sudo apt-get install libsdl2-dev
```
Compile the controller input program using SDL2 flags to link the appropriate libraries
```bash
# Compiles using g++ and links SDL2 libraries
g++ ./Controller\ Input/controller.cpp -lSDL2main -lSDL2
```
PS4 controller program requires higher privileges
```bash
# Run the command with sudo
sudo ./controller
