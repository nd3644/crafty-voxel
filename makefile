CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2 -I/usr/include/imgui/ -I/usr/include/imgui/backends/ -DENABLE_IMGUI
LDFLAGS=-lSDL2main -lSDL2 -lSDL2_image -lGL -lGLEW -L/usr/lib/imgui/ -limgui -lstb -lnoise -lgsl
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

TARGET = crafty.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run:
	./$(TARGET)