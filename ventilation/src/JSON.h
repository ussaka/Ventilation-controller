#ifndef JSON_H_
#define JSON_H_

#include <functional>
#include <string>

class JSON
{
public:
	JSON(const std::string& data);
	JSON();

	bool next(std::string& key, std::string& value);

	void add(const std::string& key, int value);
	void addLiteral(const std::string& key, const std::string& value);
	std::string toString() { return data + '}'; }

private:
	std::string data;
	size_t index = 0;
};

#endif /* JSON_H_ */
