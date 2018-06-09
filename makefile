MEMORY_STYLE := ./protobufs-default

CXX := g++
CXXFLAGS := -std=c++11 -pthread -pedantic -Wall -Wextra -Weffc++ -Werror -g -O2 -fPIC -Wno-unused-private-field
INCLUDES :=	-I/opt/local/include/ -I radiotap-library

LIBS     := -lpcap
#$(MEMORY_STYLE)/libremyprotos.a
OBJECTS  := packet-ethernet.o packet-ip.o packet-tcp.o

all: monitor

.PHONY: all

monitor: monitor.o $(OBJECTS)
	$(CXX) $(inputs) -o $(output) $(LIBS)

%.o: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(input) -o $(output)
