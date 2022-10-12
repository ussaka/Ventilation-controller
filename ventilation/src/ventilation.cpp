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
#include "JSON.h"
#include "Networking.h"
#include "NumericProperty.h"
#include "external/ITM_Wrapper.h"
#include "LcdUi.h"

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

    bool isAutomatic = false;
	int goal = 0;

	int speed = 0;
	float speedMultiplier;

	LcdUi ui;

    I2C i2c(0x40);

    ITM_Wrapper output;
	output.print("Test");

    Networking net("OnePlus Nord N10 5G", "salsasana666", "192.168.83.223");

	ModbusMaster fan(1);
	fan.begin(9600);

	ModbusMaster co2(240);
	co2.begin(9600);

	ModbusMaster hmp(241);
	hmp.begin(9600);

	ModbusRegister AO1(&fan, 0);
	AO1.write(0);

	ModbusRegister co2Data(&co2, 0x100, false);

	//	Relative humidity
	ModbusRegister humidityData(&hmp, 0x100, false);
	ModbusRegister temperatureData(&hmp, 0x101, false);

	ModbusRegister co2Status(&co2, 0x800, false);	// Absolute humidity
	ModbusRegister hmpStatus(&hmp, 0x200, false);	// Absolute humidity

	net.subscribe("controller/settings", [&](const std::string& data)
	{
    	output.print("DATA '", data, "'");
    	JSON json(data);

    	std::string key;
    	std::string value;

    	while(json.next(key, value))
		{
    		if(key == "auto")
    			isAutomatic = value == "true";

    		else if(key == "pressure")
			{
    			goal = atoi(value.c_str());

    			if(goal < 10) speedMultiplier = 1.0f;
    			else if(goal < 20) speedMultiplier = 10.0f;

    			else speedMultiplier = 100.0f;
			}

    		else if(key == "speed")
			{
    			speed = atoi(value.c_str()) * 10;
    			AO1.write(speed);
			}
		}
	});

	unsigned minRPM = 100;
	unsigned maxRPM = 1000;

	unsigned samples = 0;
	unsigned elapsed = 0;

	float result = 0;

    while(1)
    {
    	Networking::poll(10);
    	elapsed += 10;

    	if(elapsed >= 50)
    	{
			JSON status;
			i2c.write(0XF1);

			bool ok;
			const uint8_t* resp = i2c.getResponse(3, ok);

			if(ok)
			{
				//	What's the pressure according to the sensor?
				int16_t real = resp[0] << 8 | resp[1];
				result = (static_cast <float> (real) / scaleFactor) * altitudeCorrection;

				if(isAutomatic)
				{
					float relation = result / goal;
					float percentage = 1.0f - relation;

					speed += speedMultiplier * percentage;

					if(speed > maxRPM) speed = maxRPM;
					else if(speed < minRPM) speed = minRPM;

					output.print(percentage, "% -> Speed ", speed);
					AO1.write(speed);
				}
			}

			else
			{
				output.print("No response");
			}

    		//output.print("Sending status");

			status.add("nr", samples);
    		status.add("pressure", result);

    		if(isAutomatic) status.add("setpoint", goal);
    		else status.add("setpoint", static_cast <float> (speed) / 10);

    		status.add("speed", static_cast <float> (speed) / 10);
    		status.addLiteral("auto", isAutomatic ? "true" : "false");
    		status.addLiteral("error", "false");

			// FIXME Read status before reading value
    		int co2Value = co2Data.read();
    		int rhValue = humidityData.read();
    		int tempValue = temperatureData.read();

    		status.add("co2", co2Value);
    		status.add("rh", rhValue);
    		status.add("temp", tempValue);

    		net.publish("controller/status", status.toString());
			samples++;

    		elapsed = 0;
    		ui.update(isAutomatic, goal, tempValue, speed, co2Value, rhValue, result);
    	}
    }

    return 0 ;
}
