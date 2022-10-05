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

class LcdUi {
public:
	LcdUi();
	virtual ~LcdUi();
	void read_btns();
	int update(std::string val);
private:
	//buttons
	DigitalIoPin sw_a2;
	DigitalIoPin sw_a3;
	DigitalIoPin sw_a4;
	DigitalIoPin sw_a5;

	// Lcd pins
	DigitalIoPin rs;
	DigitalIoPin en;
	DigitalIoPin d4;
	DigitalIoPin d5;
	DigitalIoPin d6;
	DigitalIoPin d7;

	LiquidCrystal lcd;
	int menu_pos = 0;
	bool set_val = false;
};

#endif /* LCDUI_H_ */
