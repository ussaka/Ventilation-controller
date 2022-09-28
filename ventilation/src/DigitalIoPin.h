#ifndef DigitalIoPin_H_
#define DigitalIoPin_H_

class DigitalIoPin
{
public:
	DigitalIoPin(int port, int pin, bool input = true, bool pullup = true, bool invert = false);
	DigitalIoPin(const DigitalIoPin&) = delete;
	virtual ~DigitalIoPin();

	bool read();
	void write(bool value);
private:
	bool invert;

	int port;
	int pin;
};

#endif /* DigitalIoPin_H_ */
