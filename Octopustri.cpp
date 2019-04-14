#define UDMXVERSION "1.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <usb.h> /* this is libusb, see http://libusb.sourceforge.net/ */
#include "uDMX_cmds.h"

#include "DMXDevice.hpp"

#include "Octopustri.hpp"


Octopustri::Octopustri(usb_dev_handle *handle, unsigned char *values, int channel)
{
	this->handle = handle;
	this->channel = channel;
	this->values = values;

	DMXDevice::setValue(this->handle, this->values, 255, this->channel+3);
	DMXDevice::setValue(this->handle, this->values, 255, this->channel);
}

int Octopustri::setColor(unsigned char r, unsigned char g, unsigned char b)
{
	return 0;
}
int Octopustri::setBrightness(unsigned char b)
{

	return 0;
}
int Octopustri::setStrobe(unsigned char b)
{
	return DMXDevice::setValue(this->handle, this->values, b, this->channel+2);
}
int Octopustri::setSpeed(unsigned char b)
{
	return DMXDevice::setValue(this->handle, this->values, b, this->channel+1);
}
