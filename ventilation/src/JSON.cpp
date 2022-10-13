#include "JSON.h"

#include <cctype>

JSON::JSON(const std::string& data) : data(data)
{
	index = data.find('{');
}

JSON::JSON()
{
	data = "{";
}

void JSON::addLiteral(const std::string& key, const std::string& value)
{
	if(data.length() > 1)
		data += ',';

	data += '"' + key + '"' + ":" + value;
}


bool JSON::next(std::string& key, std::string& value)
{
	size_t keyStart = data.find('"', index + 1);
	if(keyStart == std::string::npos)
		return false;

	size_t keyEnd = data.find('"', keyStart + 1);
	size_t delimit = data.find(':', keyEnd);

	//	Skip whitespace after the delimiter
	for(index = delimit + 1; isspace(data[index]); index++);
	size_t valueStart = index;

	//	Skip until there's a comma or a closing bracket
	for(; data[index] != ',' && data[index] != '}'; index++);

	key = std::string(data.begin() + keyStart + 1, data.begin() + keyEnd);
	value = std::string(data.begin() + valueStart, data.begin() + index);

	return true;
}
