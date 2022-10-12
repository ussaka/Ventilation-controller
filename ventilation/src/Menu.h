#ifndef MENU_H_
#define MENU_H_

#include "Property.h"
#include "LiquidCrystal.h"
#include "external/ITM_Wrapper.h"

class Menu {
public:
	Menu(LiquidCrystal &lcd) :
			lcd(lcd) {
	}

	enum class Event {
		Confirm, Back, Up, Down
	};

	void send(Event event);

	bool addProperty(Property &property);
	Property* getSelected() { return properties[selected]; }

	bool isEditing() {
		return editing;
	}
	void display();

private:
	LiquidCrystal &lcd;
	void clearDisplay();

	const static unsigned maxProperties = 10;
	unsigned count = 0;

	unsigned selected = 0;
	bool editing = false;

	Property* properties[maxProperties];
};

#endif /* MENU_H_ */
