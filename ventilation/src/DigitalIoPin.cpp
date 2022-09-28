#include "DigitalIoPin.h"

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

DigitalIoPin::DigitalIoPin(int port, int pin, bool input, bool pullup, bool invert)
	: invert(invert), port(port), pin(pin)
{
	int mask = IOCON_DIGMODE_EN;

	if(pullup) mask |= IOCON_MODE_PULLUP;
	if(invert) mask |= IOCON_INV_EN;

	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, mask);
	Chip_GPIO_SetPinDIR(LPC_GPIO, port, pin, !input);
}

DigitalIoPin::~DigitalIoPin()
{
}

bool DigitalIoPin::read()
{
	return Chip_GPIO_ReadPortBit(LPC_GPIO, port, pin);
}

void DigitalIoPin::write(bool value)
{
	if(invert) value = !value;
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
}
