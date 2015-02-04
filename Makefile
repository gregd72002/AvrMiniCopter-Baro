CXX=g++
CXX_OPTS= -Wall -g -O2 -DDEBUG1
CXXFLAGS=$(CXX_OPTS)

CC=cc
CFLAGS=
CC_OPTS=-lstdc++ -lm
LDFLAGS=$(CC_OPTS)
#LDFLAGS=-lpthread -pthread -lstdc++ -lsupc++ 
#LD_OPTS=-lpthread -lrt -lstdc++

INSTALL=install

OBJ_AVRBARO=avrbaro.o routines.o bmpsensor/libbs.a libs/libi2cdev.a

%.o: %.c                                                                         
	$(CXX) -c $(CXXFLAGS) $(CXX_OPTS) $< -o $@ 

all: avrbaro

avrbaro: $(OBJ_AVRBARO)
	$(CC) $(CFLAGS) $(OBJ_AVRBARO) -o avrbaro $(LDFLAGS) $(CC_OPTS) 

bmpsensor/libbs.a:
	$(MAKE) -C bmpsensor/ 

libs/libi2cdev.a:
	$(MAKE) -C libs/i2cdev

install:
	$(INSTALL) -m 0755 -d $(DESTDIR)/usr/local/bin
	$(INSTALL) -m 755 utils/S92avrbaro $(DESTDIR)/etc/init.d/
	$(INSTALL) -m 755 utils/avrbaro.sh $(DESTDIR)/usr/local/bin/
	$(INSTALL) -m 755 avrbaro $(DESTDIR)/usr/local/bin/

clean:
	cd bmpsensor && $(MAKE) clean
	cd libs/i2cdev && $(MAKE) clean
	rm -rf avrbaro
	rm -rf *.o

