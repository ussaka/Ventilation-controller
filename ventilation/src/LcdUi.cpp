/*
 * LcdUi.cpp
 *
 *  Created on: 5 Oct 2022
 *      Author: kkivi
 */

#include "LcdUi.h"

LcdUi::LcdUi(NumericProperty<int> &mode, NumericProperty<int> &speed,
		NumericProperty<float> &pressure, NumericProperty<int> &setpoint) :
		mode(mode), speed(speed), pressure(pressure), setpoint(setpoint), buttons(
				{ { 1, 8 }, { 0, 5 }, { 0, 6 }, { 0, 7 } }), rs(0, 29,
				DigitalIoPin::output), en(0, 9, DigitalIoPin::output), d4(0, 10,
				DigitalIoPin::output), d5(0, 16, DigitalIoPin::output), d6(1, 3,
				DigitalIoPin::output), d7(0, 0, DigitalIoPin::output), lcd(&rs,
				&en, &d4, &d5, &d6, &d7), menu(lcd), temp("temp", -40, 60,
				true), co2("co2", 0, 10000, true), rh("rh", 0, 100, true) {
	lcd.begin(16, 2); // configure display geometry
	// Add menu properties

	//	These properties need to know when menu is in edit mode
	mode.addToMenu(menu);
	setpoint.addToMenu(menu);
	speed.addToMenu(menu);
	pressure.addToMenu(menu);

	//	These properties don't need to know their menu
	menu.addProperty(temp);
	menu.addProperty(rh);
	menu.addProperty(co2);

	menu.display(); // Display changes
}

void LcdUi::btnStatusUpdate(void) {
	// Check if any of the buttons were pressed
	for (int i = 0; i < 4; i++) {
		if (buttons[i].btn.read()) {
			buttons[i].isPressed = true;
		} else if (buttons[i].isPressed) {
			buttons[i].isPressed = false;

			// Handle button presses
			switch (i) {
			case 0: // sw_a2
				menu.send(Menu::Event::Down);
				break;
			case 1: // sw_a3
				menu.send(Menu::Event::Up);
				break;
			case 2: // sw_a4
				menu.send(Menu::Event::Confirm);
				if (!menu.isEditing() && onValueChange) {
					Property *selected = menu.getSelected();
					onValueChange(*selected);
				}
				break;
			case 3: // sw_a5
				menu.send(Menu::Event::Back);
				break;
			}
		}
	}
}

void LcdUi::update(int _temp, int _co2, int _rh) {
	int changes = 0;

	if (menu.isEditing()) {
		return;
	}

	// Check if values were changed outside the menu
	changes += temp.changeIfDifferent(_temp);
	changes += co2.changeIfDifferent(_co2);
	changes += rh.changeIfDifferent(_rh);

	changes += mode.isDirty();
	changes += speed.isDirty();
	changes += pressure.isDirty();
	changes += setpoint.isDirty();

	if (changes > 0)
		menu.display(); // Display changes
}

LcdUi::~LcdUi() {
	// TODO Auto-generated destructor stub
}

