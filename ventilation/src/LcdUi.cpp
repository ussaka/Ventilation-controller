/*
 * LcdUi.cpp
 *
 *  Created on: 5 Oct 2022
 *      Author: kkivi
 */

#include "LcdUi.h"

LcdUi::LcdUi() {
	// Lcd pins
	DigitalIoPin rs(0, 8, false, true, false);
	DigitalIoPin en(1, 6, false, true, false);
	DigitalIoPin d4(1, 8, false, true, false);
	DigitalIoPin d5(0, 5, false, true, false);
	DigitalIoPin d6(0, 6, false, true, false);
	DigitalIoPin d7(0, 7, false, true, false);

	LiquidCrystal _lcd(&rs, &en, &d4, &d5, &d6, &d7);

	lcd = &_lcd;
}

LcdUi::~LcdUi() {
	// TODO Auto-generated destructor stub
}

