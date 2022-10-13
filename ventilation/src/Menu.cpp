#include "Menu.h"

void Menu::send(Event event)
{
	if(editing)
	{
		switch(event)
		{
			case Event::Up: properties[selected]->input(true); break;
			case Event::Down: properties[selected]->input(false); break;

			case Event::Back: properties[selected]->stopEdit(true); editing = false; break;
			case Event::Confirm: properties[selected]->stopEdit(false); editing = false; break;
		}

		display();
		return;
	}

	switch(event)
	{
		case Event::Down:
			if(--selected >= count)
				selected = count - 1;
		break;

		case Event::Up:
			if(++selected >= count)
				selected = 0;
		break;

		case Event::Back: break;

		case Event::Confirm:
			if (!properties[selected]->isConstant()) {
				editing = true;
			}
		break;
	}

	display();
}

bool Menu::addProperty(Property& property)
{
	if(count >= maxProperties)
		return false;

	properties[count] = &property;
	count++;

	return true;
}

void Menu::display()
{
	clearDisplay();

	Property& p = *properties[selected];
	lcd.print(p.getName() + "(" + p.getRange() + ")\n" + (editing ? "* " : "") + p.getValue());
}

void Menu::clearDisplay()
{
	std::string empty(std::string(16, ' '));
	lcd.print(empty + '\n' + empty);
}
