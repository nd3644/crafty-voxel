CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2 -g -I./include/ -I/usr/include/imgui/ -I/usr/include/imgui/backends/ -DENABLE_IMGUI
LDFLAGS=-lSDL2main -lSDL2 -lSDL2_image -lGL -lGLEW -L/usr/lib/imgui/ -limgui -lstb -lnoise -lgsl
SRCS=$(wildcard src/*.cpp)
OBJS=$(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
BUILD_DIR = build

MAKEFLAGS += -j$(nproc)

.PHONY: all clean run

TARGET = $(BUILD_DIR)/crafty.out

all: $(TARGET) | $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(BUILD_DIR)

run:
	./$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)