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
private:
	LiquidCrystal *lcd;
};

#endif /* LCDUI_H_ */
