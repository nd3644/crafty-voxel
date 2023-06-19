CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2 -g -I/usr/include/imgui/ -I/usr/include/imgui/backends/
LDFLAGS=-lSDL2main -lSDL2 -lSDL2_image -lGL -lGLEW -L/usr/lib/imgui/ -limgui -lstb -lnoise
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

TARGET = crafty

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run:
	./$(TARGET)