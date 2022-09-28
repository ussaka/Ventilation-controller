#ifndef I2C_H_
#define I2C_H_

#include <cstdint>

class I2C
{
public:
	I2C(uint8_t slaveAddr);

	void write(uint8_t byte);
	const uint8_t* getResponse(uint16_t readCount, bool& ok);
	void send();

	uint16_t getStatus() { return status; }

private:
	static const unsigned bufferMax = 64;

	uint8_t txBuff[bufferMax];
	uint8_t rxBuff[bufferMax];

	uint16_t txSz = 0;
	uint16_t rxSz = 0;

	uint16_t status;
	uint8_t slaveAddr;

};

#endif /* I2C_H_ */
