#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <string>

class Property
{
public:
	Property(const std::string& name);

	const std::string& getName() { return name; }
	virtual std::string getValue() = 0;
	virtual std::string getRange() = 0;

	bool startEdit();

	virtual void stopEdit(bool discard) = 0;
	virtual void input(bool up) = 0;

protected:
	std::string name;
};

#endif /* PROPERTY_H_ */
