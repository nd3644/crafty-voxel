CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2
LDFLAGS=-lSDL2main -lSDL2 -lGL -lGLEW
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