#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>

void error(std::string msg)
{
	std::cerr << msg << ":" << strerror(errno) << std::endl;
	std::exit(EXIT_FAILURE);
}

int getCmdOptAsInt(char** first, char** last, const std::string& option)
{
	char** iter = std::find(first, last, option);
	if (iter != last && ++iter != last)
		return std::atoi(*iter);
	return 0;
}
