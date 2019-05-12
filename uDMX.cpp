/*
 * Name: uDMX.c
 * Project: Commandline utility for uDMX interface
 * Author: Markus Baertschi
 * Based on work from: Christian Starkjohann, 2005-01-16
 *
 * Creation Date: 2005-01-16
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: uDMX.c,v 1.1.1.1 2006/02/15 17:55:06 cvs Exp $
 */

/*
General Description:
This program controls the PowerSwitch USB device from the command line.
It must be linked with libusb, a library for accessing the USB bus from
Linux, FreeBSD, Mac OS X and other Unix operating systems. Libusb can be
obtained from http://libusb.sourceforge.net/.
*/

#define UDMXVERSION "1.0"

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <usb.h> /* this is libusb, see http://libusb.sourceforge.net/ */
#include <vector>
#include <stdio.h>
#include <curses.h>
#include <ctype.h>
#include <queue>
#include <fcntl.h>
#include <string>

#include <iostream>
#include <fstream>


#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>


#include "uDMX_cmds.h"


#include "DMXDevice.hpp"

#include "Octopustri.hpp"
#include "SmokeM.hpp"
#include "FireworksII.hpp"


#define USBDEV_SHARED_VENDOR 0x16C0  /* Obdev's free shared VID */
#define USBDEV_SHARED_PRODUCT 0x05DC /* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */

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


using namespace std;


int debug = 0;
int verbose = 0;

static int usbGetStringAscii(usb_dev_handle *dev, int index, int langid,
                             char *buf, int buflen)
{
	char buffer[256];
	int rval, i;

	if ((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
								(USB_DT_STRING << 8) + index, langid, buffer,
								sizeof(buffer), 1000)) < 0)
		return rval;
	if (buffer[1] != USB_DT_STRING)
		return 0;
	if ((unsigned char)buffer[0] < rval)
		rval = (unsigned char)buffer[0];
	rval /= 2;
	/* lossy conversion to ISO Latin1 */
	for (i = 1; i < rval; i++) {
		if (i > buflen) /* destination buffer overflow */
			break;
		buf[i - 1] = buffer[2 * i];
		if (buffer[2 * i + 1] != 0) /* outside of ISO Latin1 range */
			buf[i - 1] = '?';
	}
	buf[i - 1] = 0;
	return i - 1;
}

/*
 * uDMX uses the free shared default VID/PID.
 * To avoid talking to some other device we check the vendor and
 * device strings returned.
 */
static usb_dev_handle *findDevice(void)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	char string[256];
	int len;
	usb_dev_handle *handle = 0;

	usb_find_busses();
	usb_find_devices();
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == USBDEV_SHARED_VENDOR &&
				dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT) {
				if (debug) { printf("Found device with %x:%x\n",
							   USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT); }

				/* open the device to query strings */
				handle = usb_open(dev);
				if (!handle) {
					fprintf(stderr, "Warning: cannot open USB device: %s\n",
							usb_strerror());
					continue;
				}

				/* now find out whether the device actually is a uDMX */
				len = usbGetStringAscii(handle, dev->descriptor.iManufacturer,
										0x0409, string, sizeof(string));
				if (len < 0) {
					fprintf(stderr, "warning: cannot query manufacturer for device: %s\n",
							usb_strerror());
					goto skipDevice;
				}
				if (debug) { printf("Device vendor is %s\n",string); }
				if (strcmp(string, "www.anyma.ch") != 0)
					goto skipDevice;

				len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
				if (len < 0) {
					fprintf(stderr, "warning: cannot query product for device: %s\n", usb_strerror());
					goto skipDevice;
				}
				if (debug) { printf("Device product is %s\n",string); }
				if (strcmp(string, "uDMX") == 0)
					break;

			skipDevice:
				usb_close(handle);
				handle = NULL;
			}
		}
		if (handle)
			break;
	}
	return handle;
}

int maxx, maxy;

void setup()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nonl();
	raw();

	start_color();
	use_default_colors();
	init_pair(1,COLOR_CYAN,-1);
	init_pair(2,COLOR_MAGENTA,-1);
	init_pair(3,COLOR_GREEN,-1);
	init_pair(5,COLOR_YELLOW,-1);
	init_pair(4,COLOR_RED,-1);

	timeout(10);
	maxx = getmaxx(stdscr);
	maxy = getmaxy(stdscr);

	//mvprintw(maxy/2, maxx/2-11, "Starting...");
}

FILE *get_port(char *device)
{
    FILE *fp = fopen(device, "r");
    if (fp == NULL)
    {
        perror("get_port: Unable to open serial device - ");
        return NULL;
    }

    return fp;
}

struct Params
{
	unsigned char speed, strobe, smoke;
	bool blackout;
	bool *blackouts;
	bool die;
	string good;
};


void *read_serial(void *params)
{
	FILE *fp;
	int readCount = 64;
	char *buf = malloc(sizeof (char) * readCount*2);
	char *pos;

	struct Params *p = params;

	ofstream myfile;
  	myfile.open("log.txt");


	fp = get_port("/dev/ttyACM0");


	// Filter out the garbage
	fgets(buf, readCount, fp);
	fgets(buf, readCount, fp);

	while (1)
	{
		usleep(100);

		fgets(buf, readCount*2, fp);
		pos = buf;

		// We got a match!
		if (pos != NULL)
		{
			// We got POT
			if (strstr(pos, "Pot") != NULL)
			{
				pos[strlen(pos)-2] = '\0';
				myfile << "NUM: " << atoi(pos+4) << "\n";
				p->speed = (unsigned char)(((float)atoi(pos+4)/1023.0)*255.0);
				myfile << "CONV: " << (int)p->speed << "\n";
			}
			if (strstr(pos, "Pus") != NULL)
			{
				pos[strlen(pos)-2] = '\0';
				myfile << "Push: " << atoi(pos+4) << "\n";
				if (atoi(pos+4) == 1)
				{
					p->smoke = 255;
					myfile << "SMOKING\n\n\n";
				}
				else
					p->smoke = 0;
				myfile << "Smoke: " << atoi(pos+4) << "\n";
			}
			if (strstr(pos, "Tog") != NULL)
			{
				*(pos+5) = '\0';
				if (atoi(pos+4) == 0)
					p->blackout = true;
				else
					p->blackout = false;
			}
			myfile << pos;
		}


		if (p->die)
			break;

		/*printf("Yaw: %.2f; Pitch: %.2f; Roll: %.2f;\n", ypr[0],
		                                                ypr[1], ypr[2]);*/
	}
	printf("Closing Serial Thread...\n");

	fclose(fp);
	myfile.close();
	return 0;
}


int main(int argc, char **argv)
{

	setup();




	usb_dev_handle *handle = NULL;
	usb_set_debug(0);


	/*
	 * USB Initialisation
	 *
	 * If we can not find the uDMX device we exit
	 */
	usb_init();
	handle = findDevice();
	if (handle == NULL)
	{
		fprintf(stderr,
				"Could not find USB device www.anyma.ch/uDMX (vid=0x%x pid=0x%x)\n",
				USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT);
		exit(1);
	}
	//printf("Connected to uDMX Device, firing up\n");

	unsigned char currValues[512];


	// Create the devices :3
	vector<DMXDevice *> devices;
	SmokeM *smokeM = new SmokeM(handle, currValues, 40);

	Octopustri *octo = new Octopustri(handle, currValues, 1);
	FireworksII *fire = new FireworksII(handle, currValues, 32);
	devices.push_back(octo);
	devices.push_back(fire);



	bool blackoutRestored = true;
	int blackoutCount = 3;

	struct Params p;
	p.speed = 100;
	p.strobe = 0;
	p.smoke = 0;
	p.blackout = false;
	p.blackouts = malloc(sizeof (bool) * blackoutCount);
	p.die = false;


	// Setup the Arduino listener
	pthread_t serial_t;
	pthread_create(&serial_t, NULL, read_serial, &p);






	while (true)
	{

		char ch = getch();

		if (ch == 'q')
			break;

		if (ch == 's')
		{
			if (p.smoke == 0)
				p.smoke = 254;
			else
				p.smoke = 0;
		}

		unsigned char inc = 1;

		if (ch == 't')
			p.strobe += inc;
		if (ch == 'r')
			p.strobe -= inc;
		if (ch == 'w')
			p.speed -= inc;
		if (ch == 'e')
			p.speed += inc;


		if (ch == 'b')
		{
			p.speed = 101;
			p.strobe = 0;
		}
		if (ch == 'n')
		{
			p.speed = 200;
			p.strobe = 0;
		}
		if (ch == 'm')
		{
			p.speed = 200;
			p.strobe = 255;
		}


		if (ch == 'v')
			p.blackout = !p.blackout;


		// Smoke machine first
		smokeM->setSpeed(p.smoke);
		// Set all the values
		if (!p.blackout)
		{
			if (!blackoutRestored)
			{
				int nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, cmd_SetChannelRange, 512, 0, (char*)currValues, 512, 1000);
				if (nBytes < 0)
					fprintf(stderr, "USB error: %s\n", usb_strerror());

				blackoutRestored = true;
			}



			vector<DMXDevice *>::iterator it;
			int i;
			for (it = devices.begin(), i = 0; it != devices.end(); it++, i++)
			{
				DMXDevice *d = *it;
				d->setSpeed((unsigned char)p.speed);
				d->setStrobe((unsigned char)p.strobe);
				d->blackout(p.blackouts[i]);
			}
		}
		else
		{
			blackoutRestored = false;

			unsigned char values[512];
			for (int i = 0; i < 512; i++)
				values[i] = 0;

			int nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, cmd_SetChannelRange, 512, 0, (char*)values, 512, 1000);
			if (nBytes < 0)
				//fprintf(stderr, "USB error: %s\n", usb_strerror());
				;
		}


		// Show 'em what we got
		mvprintw(1, 5, "Smoke: %d         ", p.smoke);
		mvprintw(2, 5, "Strobe: %d         ", p.strobe);
		mvprintw(3, 5, "Speed: %d         ", p.speed);
		mvprintw(6, 5, "Binary Speed: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(p.speed));
		mvprintw(4, 5, "Blackout: ");
		if (p.blackout)
			mvprintw(4, 16, "True ");
		else
			mvprintw(4, 16, "False");
		refresh();
	}



	endwin();
	usb_close(handle);
	printf("Dead\n");
	p.die = true;
	pthread_join(serial_t, NULL);
	printf("Joined\n");
	return 0;
}
