#############################################################################
#
# Makefile for nrf24 server
#
# License: GPL (General Public License)
# Author:  Erik Svensson <erik.public@gmail.com>
# Date:    2013-07
#

prefix := /opt/rpi-nrf24

# The recommended compiler flags for the Raspberry Pi
CC=g++
CCFLAGS=-c -Wall -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
LDFLAGS=

SOURCES=server.cpp sleep.cpp gpio.cpp spi.cpp nrf24.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS)

