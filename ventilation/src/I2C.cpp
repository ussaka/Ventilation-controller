#include "I2C.h"

#include "chip.h"
#include "i2cm_15xx.h"

#define I2C_CLK_DIVIDER         (40)
/* 100KHz I2C bit-rate */
#define I2C_BITRATE         (100000)
/* Standard I2C mode */
#define I2C_MODE    (0)

static void Init_I2C_PinMux()
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
}

/* Setup I2C handle and parameters */
static void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_I2C_Init(LPC_I2C0);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(LPC_I2C0, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(LPC_I2C0, I2C_BITRATE);

	/* Enable Master Mode */
	Chip_I2CM_Enable(LPC_I2C0);
}

I2C::I2C(uint8_t slaveAddr) : slaveAddr(slaveAddr)
{
	static bool initialized = false;

	if(!initialized)
	{
		/*	This could take a pin and a port as a parameter
		 * 	so that this class can be used with other pins
		 * 	other than the hardcoded ones */
		Init_I2C_PinMux();

		setupI2CMaster();
		NVIC_DisableIRQ(I2C0_IRQn);

		initialized = true;
	}
}

void I2C::write(uint8_t byte)
{
	txBuff[txSz] = byte;
	txSz++;
}

void I2C::send()
{
	bool ok;
	getResponse(0, ok);
}

const uint8_t* I2C::getResponse(uint16_t readCount, bool& ok)
{
	I2CM_XFER_T i2cmXferRec;
	rxSz = readCount;

	i2cmXferRec.slaveAddr = slaveAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSz;
	i2cmXferRec.rxSz = rxSz;
	i2cmXferRec.txBuff = txBuff;;
	i2cmXferRec.rxBuff = rxBuff;

	Chip_I2CM_XferBlocking(LPC_I2C0, &i2cmXferRec);
	status = i2cmXferRec.status;

	if(status != I2CM_STATUS_OK)
	{
		ok = false;
		return nullptr;
	}

	txSz = 0;
	ok = true;
	return rxBuff;
}
