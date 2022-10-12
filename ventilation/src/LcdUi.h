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
	LcdUi(NumericProperty <int>& mode, NumericProperty <int>& speed, NumericProperty <float>& pressure, NumericProperty <int>& setpoint);
	virtual ~LcdUi();
	void read_btns();
	void update(int _temp, int _co2, int _rh);
	void btnStatusUpdate(void);

private:
	NumericProperty <int>& mode;
	NumericProperty <int>& speed;
	NumericProperty <float>& pressure;
	NumericProperty <int>& setpoint;



	struct button {
		button(int port, int pin) :
				btn(port, pin, DigitalIoPin::pullup, true) {
		}
		;
		DigitalIoPin btn;
		bool isPressed = false;
	};

	// Buttons
	button buttons[4];

	// Lcd display pins
	DigitalIoPin rs;
	DigitalIoPin en;
	DigitalIoPin d4;
	DigitalIoPin d5;
	DigitalIoPin d6;
	DigitalIoPin d7;

	LiquidCrystal lcd;

	Menu menu;

	NumericProperty<float> temp;
	NumericProperty<float> co2;
	NumericProperty<float> rh;
};

#endif /* LCDUI_H_ */
