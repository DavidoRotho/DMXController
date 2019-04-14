class SmokeM : public DMXDevice
{
private:
	usb_dev_handle *handle;
	int channel;
	unsigned char *values;

public:
	int setColor(unsigned char r, unsigned char g, unsigned char b);
	int setBrightness(unsigned char b);
	int setStrobe(unsigned char b);
	int setSpeed(unsigned char b);

	SmokeM(usb_dev_handle *handle, unsigned char *values, int channel);
};
