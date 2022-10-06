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
#include "Menu.h"
#include "Networking.h"
#include "NumericProperty.h"
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

#include "systick.h"

uint32_t millis() { return systicks; }
uint32_t get_ticks() { return systicks; }

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
	output.print("Test");

    Networking net("OnePlus Nord N10 5G", "salsasana666", "192.168.83.223");

    net.subscribe("controller/settings", [&output](const std::string& data)
	{
	});

	ModbusMaster fan(1);
	fan.begin(9600);

	ModbusRegister AO1(&fan, 0);
	ModbusRegister DI1(&fan, 4, false);

	ModbusMaster co2(240);
	co2.begin(9600);

	ModbusMaster hmp(241);
	hmp.begin(9600);

	ModbusRegister co2Data(&co2, 0x100, false);

	//	Relative humidity
	ModbusRegister humidityData(&hmp, 0x100, false);
	ModbusRegister temperatureData(&hmp, 0x101, false);

	ModbusRegister co2Status(&co2, 0x800, false);	// Absolute humidity
	ModbusRegister hmpStatus(&hmp, 0x200, false);	// Absolute humidity

	const int co2Ok = 0;
	const int hmpOk = 1;

	unsigned index = 0;
	bool right = true;

	const float goal = 60;
	float speed = 100;

	AO1.write(0);
	Sleep(1000);

    while(1)
    {
    	Networking::poll();

		AO1.write(0);
		continue;

    	i2c.write(0XF1);

    	bool ok;
    	const uint8_t* resp = i2c.getResponse(3, ok);

    	if(ok)
    	{
    		int16_t real = resp[0] << 8 | resp[1];
    		float result = (static_cast <float> (real) / scaleFactor) * altitudeCorrection;

			output.print("Result ", result);

			if(result < goal)
			{
				speed += 5;
				AO1.write(speed);
			}

			else
			{
				speed -= 5;
				AO1.write(speed);
			}
    	}

    	else
    	{
    		output.print("No response");
    	}

    	Sleep(25);
    }

    return 0 ;
}
