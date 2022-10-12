#ifndef NUMERICPROPERTY_H_
#define NUMERICPROPERTY_H_

#include "Property.h"
#include "Menu.h"

#include <type_traits>
#include <sstream>
#include <cmath>

template<typename T>
class NumericProperty: public Property {
public:
	NumericProperty(const std::string &name, T min, T max,
			bool constant = false, T increment = 1) :
			Property(name, constant), min(min), max(max), increment(increment) {
		value = min;
		oldValue = value;
	}

	std::string getValue() override {
		return oneDec(value);
	}
	std::string getRange() override {
		return oneDec(min) + "-" + oneDec(max);
	}

	T getRealValue() {
		return oldValue;
	}

	bool changeIfDifferent(T newValue)
	{
		if(newValue != value)
		{
			value = newValue;
			oldValue = value;

			return true;
		}

		return false;
	}

	void stopEdit(bool discard) override
	{
		if (discard)
			value = oldValue;

		else oldValue = value;
	}

	void input(bool up) override
	{
		T inc = up ? increment : -increment;
		value += inc;

		if (value < min)
			value = min;
		if (value > max)
			value = max;
	}

	void setValue(int val) {
		oldValue = val;

		if(menu && !menu->isEditing())
			value = val;

		dirty = true;
	}

private:
	std::string oneDec(T value) {
		std::string str = std::to_string(value);
		if (std::is_same<T, float>::value || std::is_same<T, double>::value) {
			size_t dot = str.find('.');
			str.erase(str.begin() + dot + 2, str.end());
		}

		return str;
	}

	T min;
	T max;

	T value;
	T oldValue;

	T increment;
};

#endif /* NUMERICPROPERTY_H_ */
