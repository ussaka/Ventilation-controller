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
		if (menu_pos > 4) {
			menu_pos = 0;
		}
	} else if (sw_a3.read()) {
		menu_pos--;
		if (menu_pos < 0) {
			menu_pos = 0;
		}
	} else if (sw_a4.read()) {
		set_val = true;
	} else if (sw_a5.read()) {
		set_val = false;
	}
}

int LcdUi::update(std::string val) {
	LcdUi::read_btns();

	lcd.clear();
	lcd.setCursor(0, 0);

	int val_int = 0;
	if (set_val) {
		if (menu_pos == 1) {
			val_int = stoi(val);
			if (sw_a2.read()) {
				val_int--;
			}
			if (sw_a3.read()) {
				val_int++;
			}
			lcd.print("SET FAN: " + std::to_string(val_int));
			return val_int;
		}
	} else {
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
	return 0;
}

LcdUi::~LcdUi() {
	// TODO Auto-generated destructor stub
}

