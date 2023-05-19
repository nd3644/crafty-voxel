CXX=g++
CXXFLAGS=-std=c++11 -Wall -Og
LDFLAGS=-lSDL2main -lSDL2 -lSDL2_image -lGL -lGLEW
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

TARGET = crafty

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run:
	./$(TARGET)