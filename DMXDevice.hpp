class DMXDevice
{
public:
	virtual int setColor(unsigned char r, unsigned char g, unsigned char b)
	{
		printf("Default function\n");
		return -1;
	}
	virtual int setBrightness(unsigned char b)
	{
		printf("Default function\n");
		return -1;
	}
	virtual int setStrobe(unsigned char b)
	{
		printf("Default function\n");
		return -1;
	}
	virtual int setSpeed(unsigned char b)
	{
		printf("Default function\n");
		return -1;
	}
	virtual int blackout(bool black)
	{
		//printf("Default function\n");
		return -1;
	}


	int setValues(usb_dev_handle *handle, unsigned char *masterValues, unsigned char *values, int channelCount, int startChannel);
	int setValue(usb_dev_handle *handle, unsigned char *masterValues, unsigned char value, int startChannel);
};
