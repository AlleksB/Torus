CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17
SRCS = $(wildcard *.cpp)
TARGET = torus-1.0.exe

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
