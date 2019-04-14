#define UDMXVERSION "1.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <usb.h> /* this is libusb, see http://libusb.sourceforge.net/ */
#include "uDMX_cmds.h"

#include "DMXDevice.hpp"

#include "FireworksII.hpp"


FireworksII::FireworksII(usb_dev_handle *handle, unsigned char *values, int channel)
{
	this->handle = handle;
	this->channel = channel;
	this->values = values;
	DMXDevice::setValue(this->handle, this->values, 200, this->channel);
	DMXDevice::setValue(this->handle, this->values, 40, this->channel+6);

}

int FireworksII::setColor(unsigned char r, unsigned char g, unsigned char b)
{
	return 0;
}
int FireworksII::setBrightness(unsigned char b)
{
	unsigned char value = 0;
	if (b < 255/2)
		value = 0;
	else
		value = 255;

	return DMXDevice::setValue(this->handle, this->values, value, this->channel);;
}
int FireworksII::setStrobe(unsigned char b)
{
	return DMXDevice::setValue(this->handle, this->values, b, this->channel+7);
}

int FireworksII::setSpeed(unsigned char b)
{
	DMXDevice::setValue(this->handle, this->values, b, this->channel+2);
	return DMXDevice::setValue(this->handle, this->values, b, this->channel+4);
}
