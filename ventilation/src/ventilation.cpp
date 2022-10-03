/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "I2C.h"
#include "external/ITM_Wrapper.h"

#include <modbus/ModbusMaster.h>
#include <modbus/ModbusRegister.h>

#include <cr_section_macros.h>
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif

std::atomic_int counter;
static volatile uint32_t systicks;

void SysTick_Handler(void)
{
	systicks++;
	if(counter > 0)
		counter--;
}

#ifdef __cplusplus
}
#endif

uint32_t millis()
{
	return systicks;
}

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0)
		__WFI();
}

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    Board_Init();
#endif
#endif
	uint32_t sysTickRate;
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / 1000);

	const int scaleFactor = 240;
	const float altitudeCorrection = 0.95f;

    I2C i2c(0x40);
    ITM_Wrapper output;

	ModbusMaster node(1); // Create modbus object that connects to slave id 1
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	ModbusRegister AO1(&node, 0);
	ModbusRegister DI1(&node, 4, false);

	const uint16_t fa[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	output.print("Test");
	unsigned index = 0;
	bool right = true;

    while(1)
    {
		AO1.write(5 * 100);

		index += right ? +1 : -1;
		if(index >= 10 || index == 0) right = !right;

    	i2c.write(0XF1);

    	bool ok;
    	const uint8_t* resp = i2c.getResponse(3, ok);

    	if(ok)
    	{
    		int16_t real = resp[0] << 8 | resp[1];
    		float result = (static_cast <float> (real) / scaleFactor) * altitudeCorrection;

			char buf[16];
			output.print("Result ", result);
			Board_UARTPutSTR(buf);
    	}

    	else
    	{
    		output.print("No response");
    	}

    	Sleep(1000);
    }

    return 0 ;
}
