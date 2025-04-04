# Name of the output binary
TARGET = controller_linux

# Source files
SRCS = controller_linux.cpp environment_manager.cpp serial_port.cpp

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# SDL2 include and library paths (use `sdl2-config` to get them)
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDLIBS := $(shell sdl2-config --libs)

# Final build rule
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -o $@ $^ $(SDL_LDLIBS)

# Clean up build files
clean:
	rm -f $(TARGET)

