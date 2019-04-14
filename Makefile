# Name: Makefile
# Project: PowerSwitch
# Author: Christian Starkjohann
# Creation Date: 2005-01-16
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile,v 1.1.1.1 2006/02/15 17:55:06 cvs Exp $


# Concigure the following definitions according to your system. The powerSwitch
# tool has been successfully compiled on Mac OS X, Linux and Windows.

CC              = g++
LIBUSB_CONFIG   = libusb-config
# Make sure that libusb-config is in the search path or specify a full path.
# On Windows, there is no libusb-config and you must configure the options
# below manually. See examples.
CFLAGS          = `$(LIBUSB_CONFIG) --cflags` -O -Wall -g -fpermissive
#CFLAGS          = -I/usr/local/libusb/include
# On Windows replace `$(LIBUSB_CONFIG) --cflags` with appropriate "-I..."
# option to ensure that usb.h is found
LIBS            = `$(LIBUSB_CONFIG) --libs` -lncurses -lpthread
#LIBS            = `$(LIBUSB_CONFIG) --libs` -framework CoreFoundation
# You may need "-framework CoreFoundation" on Mac OS X and Darwin.
#LIBS            = -L/usr/local/libusb/lib/gcc -lusb -lpthreads
# On Windows use somthing similar to the line above.
FILES		= uDMX.o DMXDevice.o Octopustri.o FireworksII.o SmokeM.o

all: uDMX

.cpp.o:
	$(CC) $(CFLAGS) -c $<

uDMX: $(FILES)
	$(CC) -o uDMX $(FILES) $(LIBS)

clean:
	rm -f *.o
	rm -f uDMX uDMX.exe
