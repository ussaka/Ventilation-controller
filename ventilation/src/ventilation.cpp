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

#include <cr_section_macros.h>
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif

std::atomic_int counter;

void SysTick_Handler(void)
{
	if(counter > 0)
		counter--;
}

#ifdef __cplusplus
}
#endif

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

    while(1)
    {
    	i2c.write(0XF1);

    	bool ok;
    	const uint8_t* resp = i2c.getResponse(3, ok);

    	if(ok)
    	{
    		Board_UARTPutSTR("Response\r\n");
    		int16_t real = resp[0] << 8 | resp[1];
    		float result = (static_cast <float> (real) / scaleFactor) * altitudeCorrection;

			char buf[16];
			snprintf(buf, 16, "%.2f\r\n", result);
			Board_UARTPutSTR(buf);
    	}

    	else
    	{
    		Board_UARTPutSTR("No response\r\n");
    	}

    	Sleep(500);
    }

    return 0 ;
}
