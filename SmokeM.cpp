#define UDMXVERSION "1.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <usb.h> /* this is libusb, see http://libusb.sourceforge.net/ */
#include "uDMX_cmds.h"

#include "DMXDevice.hpp"

#include "SmokeM.hpp"


SmokeM::SmokeM(usb_dev_handle *handle, unsigned char *values, int channel)
{
	this->handle = handle;
	this->channel = channel;
	this->values = values;
}


int SmokeM::setColor(unsigned char r, unsigned char g, unsigned char b)
{
	return 0;
}
int SmokeM::setBrightness(unsigned char b)
{

	return 0;
}
int SmokeM::setStrobe(unsigned char b)
{

	return 0;
}

int SmokeM::setSpeed(unsigned char b)
{
	DMXDevice::setValue(this->handle, this->values, b, this->channel);

	return 0;
}
