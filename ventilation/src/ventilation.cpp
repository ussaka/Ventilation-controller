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
#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif

std::atomic_int counter;
static volatile uint32_t systicks;

void SysTick_Handler(void) {
	systicks++;
	if (counter > 0)
		counter--;
}

#ifdef __cplusplus
}
#endif

#include "systick.h"

uint32_t millis()
{
	return systicks;
}

uint32_t get_ticks()
{
	return systicks;
}

void Sleep(int ms) {
	counter = ms;
	while (counter > 0)
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

	int goal = 0;

	int speed = 0;

	const float defaultSpeedMultiplier = 10.0f;
	float speedMultiplier = 0.0f;
	unsigned stalls = 0;

	I2C i2c(0x40);

	ITM_Wrapper output;
	output.print("Test");

	//	Connect to the MQTT broker
	//Networking net("OnePlus Nord N10 5G", "salsasana666", "192.168.91.223");
	Networking net("SmartIotMQTT", "SmartIot", "192.168.1.254");

	ModbusMaster fan(1);
	fan.begin(9600);

	ModbusMaster co2(240);
	co2.begin(9600);

	ModbusMaster hmp(241);
	hmp.begin(9600);

	ModbusRegister AO1(&fan, 0);
	AO1.write(0);

	ModbusRegister co2Status(&co2, 0x800, true);
	ModbusRegister hmpStatus(&hmp, 0x200, true);

	ModbusRegister co2Data(&co2, 0x100, true);

	//	Relative humidity
	ModbusRegister humidityData(&hmp, 0x100, true);
	ModbusRegister temperatureData(&hmp, 0x101, true);

	NumericProperty <int> isAutomatic("mode", 0, 1);
	NumericProperty <int> speedProp("speed", 0, 100, true);
	NumericProperty <int> pressureProp("pressure", 0, 130, true);
	NumericProperty <int> setpointProp("setpoint", 0, 120, false, 5);

	isAutomatic.setValue(0);
	LcdUi ui(isAutomatic, speedProp, pressureProp, setpointProp);

	//	What happens when the user changed a value through the LCD
	ui.onValueChange = [&](Property& property)
	{
		bool setSetpoint = false;

		//	If mode was changed, read setpoint
		if(&property == &isAutomatic)
			setSetpoint = true;

		//	If setpoint or mode was changed, update values
		if(setSetpoint || &property == &setpointProp)
		{
			//	Update the goal
			if(isAutomatic.getRealValue())
			{
				goal = setpointProp.getRealValue();
				speedMultiplier = defaultSpeedMultiplier;
				stalls = 0;
			}

			//	Update the speed
			else
			{
				speed = setpointProp.getRealValue();
				speedProp.setValue(speed);
				AO1.write(speed * 10);
			}
		}
	};

	net.subscribe("controller/settings", [&](const std::string& data)
	{
    	JSON json(data);

		std::string key;
		std::string value;

    	//	Parse the received JSON
		while (json.next(key, value)) {

			//	Should automatic mode be set?
			if (key == "auto")
				isAutomatic.setValue(value == "true");

			//	Should goal pressure be set
			else if (key == "pressure") {
				goal = atoi(value.c_str());
				setpointProp.setValue(goal);
				speedMultiplier = defaultSpeedMultiplier;
				stalls = 0;
			}

			//	Should fan speed be set
			else if (key == "speed") {
				speed = atoi(value.c_str());
				speedProp.setValue(speed);
				setpointProp.setValue(speed);
				AO1.write(speed * 10);
			}
		}
	});

	const int minRPM = 0;
	const int maxRPM = 1000;

	unsigned samples = 0;

	unsigned elapsed = 0;
	float result = 0;

    while(1)
    {
    	//	Poll for MQTT traffic
    	Networking::poll(50);
    	elapsed += 50;

		bool ok;

		//	Read the pressure sensor
		i2c.write(0XF1);
		const uint8_t* resp = i2c.getResponse(3, ok);

		//	Did we get a response?
		if(ok)
		{
			//	What's the pressure according to the sensor?
			int16_t real = resp[0] << 8 | resp[1];
			result = (static_cast<float>(real) / scaleFactor) * altitudeCorrection;
			pressureProp.setValue(result);

			//	Should we automatically adjust the fan speed to adjust the pressure?
			if(isAutomatic.getRealValue())
			{
				if(goal > 0)
				{
					/*	To know which direction the pressure is moving, let's first calculate
					 * 	a relation between the result and the goal. The result will always
					 * 	be < 1 if the pressure is below the goal, and > 1 if the pressure is
					 * 	above the goal */
					float relation = result / goal;
					float percentage = 1.0f - relation;

					//	Let's use the percentage and a multiplier to get an exponential growth
					int change = round(speedMultiplier * percentage);
					speed += change;

					//	Clamp the RPM
					if(speed > maxRPM) speed = maxRPM;
					else if(speed < minRPM) speed = minRPM;

					//	Update the speed property
					speedProp.setValue(speed / 10);

					//	Set the fan speed
					AO1.write(speed);

					/*	With the real ventilation system the fan is is so powerful that
					 * 	we have to decrease the speed multiplier over time. It seems
					 * 	that this happens mostly with value below 50, so when
					 * 	there are enough stalls, try to halve the multiplier */
					if(goal < 50 && abs(goal - result) > 3)
					{
						if(++stalls > 60)
						{
							speedMultiplier = std::max(2.0, ceil(speedMultiplier / 2));
							stalls = 0;
						}
					}
				}

				//	The goal is 0 so turn off the fan
				else AO1.write(0);
			}
		}

		else output.print("No response");

		//	Gather samples every 50 milliseconds
    	if(elapsed >= 500)
    	{
			JSON status;

			/*	Now let's add everything necessary to a JSON and
			 * 	publish it to controller/status */

			status.add("nr", samples);
			status.add("pressure", pressureProp.getRealValue());

			status.add("setpoint", setpointProp.getRealValue());

			status.add("speed", speedProp.getRealValue());
			status.addLiteral("auto", isAutomatic.getRealValue() ? "true" : "false");
			status.addLiteral("error", "false");

			int co2Value = 0;
			int rhValue = 0;
			int tempValue = 0;

			Sleep(5);

			if(hmpStatus.read())
			{
				Sleep(5);
				tempValue = temperatureData.read() / 10;

				Sleep(5);
				rhValue = humidityData.read() / 10;
			}

			Sleep(5);
			if(co2Status.read() == 0)
			{
				Sleep(5);
				co2Value = co2Data.read();
			}


			status.add("co2", co2Value);
			status.add("rh", rhValue);
			status.add("temp", tempValue);

			//	Publish the status JSON
			net.publish("controller/status", status.toString());

			//	The sensor polling takes about 25 ms if it succeeds
			elapsed = 25;
			samples++;

			//	Update some values on the LCD
			ui.update(tempValue, co2Value, rhValue);
		}

    	//	Check for LCD UI button presses
		ui.btnStatusUpdate();
	}

	return 0;
}
