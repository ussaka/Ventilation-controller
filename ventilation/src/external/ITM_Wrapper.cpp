#include "ITM_Wrapper.h"
#include "ITM_write.h"

ITM_Wrapper::ITM_Wrapper()
{
	static bool initialized = false;

	if(!initialized)
	{
		ITM_init();
		initialized = true;
	}
}

void ITM_Wrapper::addToBuffer(const std::string& value)
{
	buffer += value;
}

void ITM_Wrapper::print()
{
	buffer += '\n';
	ITM_write(buffer.c_str());
	buffer = "";
}
