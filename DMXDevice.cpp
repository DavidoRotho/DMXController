#define UDMXVERSION "1.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <usb.h> /* this is libusb, see http://libusb.sourceforge.net/ */
#include "uDMX_cmds.h"

#include "DMXDevice.hpp"

#define USBDEV_SHARED_VENDOR 0x16C0  /* Obdev's free shared VID */
#define USBDEV_SHARED_PRODUCT 0x05DC /* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */


#include <curses.h>

 #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
 #define BYTE_TO_BINARY(byte)  \
   (byte & 0x80 ? '1' : '0'), \
   (byte & 0x40 ? '1' : '0'), \
   (byte & 0x20 ? '1' : '0'), \
   (byte & 0x10 ? '1' : '0'), \
   (byte & 0x08 ? '1' : '0'), \
   (byte & 0x04 ? '1' : '0'), \
   (byte & 0x02 ? '1' : '0'), \
   (byte & 0x01 ? '1' : '0')

int DMXDevice::setValues(usb_dev_handle *handle, unsigned char *masterValues, unsigned char *values, int channelCount, int startChannel)
{
	/* usb request for cmd_SetChannelRange:
		bmRequestType:	ignored by device, should be USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
		bRequest:		cmd_SetChannelRange
		wValue:			number of channels to set [1 .. 512-wIndex]
		wIndex:			index of first channel to set [0 .. 511]
		wLength:		length of data, must be >= wValue
	*/
	for (int i = 0; i < channelCount; i++)
		masterValues[i+startChannel-1] = values[i];

	int nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, cmd_SetChannelRange, channelCount, startChannel-1, values, channelCount, 1000);
	//int nBytes = -1;
	if (nBytes < 0)
		fprintf(stderr, "USB error: %s\n", usb_strerror());

	return nBytes;
}
int DMXDevice::setValue(usb_dev_handle *handle, unsigned char *masterValues, unsigned char value, int startChannel)
{
	/* usb request for cmd_SetSingleChannel:
		bmRequestType:	ignored by device, should be USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
		bRequest:		cmd_SetSingleChannel
		wValue:			value of channel to set [0 .. 255]
		wIndex:			channel index to set [0 .. 511]
		wLength:		ignored
	*/

	//printf("Setting %d on %d\n", value, startChannel);


	masterValues[startChannel-1] = value;

	mvprintw(startChannel+5, 30, "Setting Value for channel %d: "BYTE_TO_BINARY_PATTERN, startChannel, BYTE_TO_BINARY(value));

	int nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, cmd_SetSingleChannel, value, startChannel-1, NULL, 0, 1000);
	if (nBytes < 0)
		fprintf(stderr, "USB error: %s\n", usb_strerror());

	return nBytes;
}
