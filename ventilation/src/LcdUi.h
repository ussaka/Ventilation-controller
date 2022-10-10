/*
 * LcdUi.h
 *
 *  Created on: 5 Oct 2022
 *      Author: kkivi
 */

#ifndef LCDUI_H_
#define LCDUI_H_

#include "DigitalIoPin.h"
#include "LiquidCrystal.h"
#include "NumericProperty.h"
#include "Menu.h"

class LcdUi {
public:
	LcdUi();
	virtual ~LcdUi();
	void read_btns();
	void update(bool &_mode, int &_goal, int _temp, int _speed, int _co2, int _rh, int _pressure);
private:
	struct button {
		button(int port, int pin) :
				btn(port, pin, DigitalIoPin::pullup, true) {
		}
		;
		DigitalIoPin btn;
		bool isPressed = false;
	};

	//buttons
	button buttons[4];

	// Lcd pins
	DigitalIoPin rs;
	DigitalIoPin en;
	DigitalIoPin d4;
	DigitalIoPin d5;
	DigitalIoPin d6;
	DigitalIoPin d7;

	LiquidCrystal lcd;

	Menu menu;
	NumericProperty<int> mode;
	NumericProperty<int> setpoint;
	NumericProperty<float> temp;
	NumericProperty<float> speed;
	NumericProperty<float> co2;
	NumericProperty<float> rh;
	NumericProperty<float> pressure;
};

#endif /* LCDUI_H_ */
