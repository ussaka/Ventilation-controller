#ifndef ITM_WRAPPER_H_
#define ITM_WRAPPER_H_

#include <string>
#include <type_traits>

class ITM_Wrapper
{
public:
	ITM_Wrapper();

	template <typename...Args>
	void print(const std::string& value, const Args& ...args)
	{
		addToBuffer(value);
		print(args...);
	}

	/*	This is probably pointless because const char* should
	 *	implicitly convert to std::string but let's have it anyways :-) */
	template <typename...Args>
	void print(const char* value, const Args& ...args)
	{
		//	Unnecessary std::string created here >:(
		addToBuffer(value);
		print(args...);
	}

	template <typename T, typename...Args>
	void print(const T& value, const Args& ...args)
	{
		addToBuffer(std::to_string(value));
		print(args...);
	}

	void print();

private:
	void addToBuffer(const std::string& value);
	std::string buffer;
};

#endif /* ITM_WRAPPER_H_ */
