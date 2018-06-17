MEMORY_STYLE := ./protobufs-default

CXX := g++
CXXFLAGS := -std=c++11 -pthread -pedantic -Weffc++ -Wall -Wextra -Werror -g -O2 -fPIC -Wno-unused-private-field
INCLUDES :=	-I/opt/local/include/  -I radiotap-library

LIBS     := -lpcap 
#$(MEMORY_STYLE)/libremyprotos.a
OBJECTS  := packet-ethernet.o packet-ip.o packet-tcp.o sequence.o tcp-connection.o analyze-tcp-streams.o

all: monitor

.PHONY: all

test: test-sequence

monitor: monitor.o $(OBJECTS)
	$(CXX) $(inputs) -o $(output) $(LIBS)

test-sequence: test-sequence.o sequence.o
  $(CXX) $(inputs) -o $(output) $(LIBS)

%.o: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(input) -o $(output)
