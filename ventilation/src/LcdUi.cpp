/*
 * LcdUi.cpp
 *
 *  Created on: 5 Oct 2022
 *      Author: kkivi
 */

#include "LcdUi.h"

LcdUi::LcdUi() :
		buttons( { { 1, 8 }, { 0, 5 }, { 0, 6 }, { 0, 7 } }), rs(0, 29,
				DigitalIoPin::output), en(0, 9, DigitalIoPin::output), d4(0, 10,
				DigitalIoPin::output), d5(0, 16, DigitalIoPin::output), d6(1, 3,
				DigitalIoPin::output), d7(0, 0, DigitalIoPin::output), lcd(&rs,
				&en, &d4, &d5, &d6, &d7), menu(lcd), mode("mode", 0, 1), setpoint(
				"setpoint", 0, 120), temp("temp", -30, 30, true), speed("speed",
				0, 100, true), co2("co2", 0, 100, true), rh("rh", 0, 100, true), pressure(
				"pressure", 0, 100, true) {
	// configure display geometry
	lcd.begin(16, 2);
	menu.addProperty(mode);
	menu.addProperty(setpoint);
	menu.addProperty(temp);
	menu.addProperty(speed);
	menu.addProperty(co2);
	menu.addProperty(rh);
	menu.addProperty(pressure);

	menu.display();
}

void LcdUi::update(bool &_mode, int &_goal, int _temp, int _speed, int _co2,
		int _rh, int _pressure) {
	int changes = 0;

	changes += mode.changeIfDifferent(_mode);
	changes += mode.changeIfDifferent(_goal);
	changes += mode.changeIfDifferent(_temp);
	changes += mode.changeIfDifferent(_speed);
	changes += mode.changeIfDifferent(_co2);
	changes += mode.changeIfDifferent(_rh);
	changes += mode.changeIfDifferent(_pressure);

	if (changes > 0)
		menu.display();

	for (int i = 0; i < 4; i++) {
		if (buttons[i].btn.read()) {
			buttons[i].isPressed = true;
		} else if (buttons[i].isPressed) {
			buttons[i].isPressed = false;

			switch (i) {
			case 0: // sw_a2
				menu.send(Menu::Event::Up);
				break;
			case 1: // sw_a3
				menu.send(Menu::Event::Down);
				break;
			case 2: // sw_a4
				menu.send(Menu::Event::Confirm);
				break;
			case 3: // sw_a5
				menu.send(Menu::Event::Back);
				break;
			}
		}
	}
	_mode = mode.getRealValue();
	_goal = setpoint.getRealValue();
}

LcdUi::~LcdUi() {
	// TODO Auto-generated destructor stub
}

