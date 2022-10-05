/*
 * LcdUi.cpp
 *
 *  Created on: 5 Oct 2022
 *      Author: kkivi
 */

#include "LcdUi.h"

LcdUi::LcdUi() :
		sw_a2(1, 8, DigitalIoPin::pullup, true), sw_a3(0, 5,
				DigitalIoPin::pullup, true), sw_a4(0, 6, DigitalIoPin::pullup,
				true), sw_a5(0, 7, DigitalIoPin::pullup, true), rs(0, 29,
				DigitalIoPin::output), en(0, 9, DigitalIoPin::output), d4(0, 10,
				DigitalIoPin::output), d5(0, 16, DigitalIoPin::output), d6(1, 3,
				DigitalIoPin::output), d7(0, 0, DigitalIoPin::output), lcd(&rs,
				&en, &d4, &d5, &d6, &d7) {
	// configure display geometry
	lcd.begin(16, 2);
}

void LcdUi::read_btns() {
	if (sw_a2.read()) {
		menu_pos++;
	} else if (sw_a3.read()) {
		menu_pos--;
	}
}

void LcdUi::update(std::string val) {
	lcd.clear();
	lcd.setCursor(0, 0);
	switch (menu_pos) {
	case 0:
		lcd.print("MODE: " + val);
		break;
	case 1:
		lcd.print("FAN: " + val);
		break;
	case 2:
		lcd.print("CO2: " + val);
	case 3:
		lcd.print("RH: " + val);
	case 4:
		lcd.print("TEMP: " + val);
	default:
		lcd.print("ERROR");
		break;
	}
}

LcdUi::~LcdUi() {
	// TODO Auto-generated destructor stub
}

