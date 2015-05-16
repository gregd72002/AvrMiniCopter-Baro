CXX=g++
AR=ar
CXXFLAGS=-Wall -g -O2
CXX_OPTS=-I../libs/ -c -DMS_DEBUGO

SOURCES=ms5611.cpp
OBJECTS=$(SOURCES:.cpp=.o)

LIB=libms5611.a

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXX_OPTS) $< -o $@ 
	

$(LIB): $(OBJECTS) 
	$(AR) rcs $(LIB) $(OBJECTS) 

all: $(LIB)

clean:
	rm -rf *.o
	rm -rf $(LIB)
