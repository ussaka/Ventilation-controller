#include "Property.h"
#include "Menu.h"

Property::Property(const std::string& name, bool constant) : name(name), constant(constant)
{
}

void Property::addToMenu(Menu& which)
{
	if(which.addProperty(*this))
		menu = &which;
}

bool Property::isDirty()
{
	bool was = dirty;
	dirty = false;

	return was;
}
