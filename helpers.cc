#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>

void error(std::string msg)
{
	std::cerr << msg << ":" << strerror(errno) << std::endl;
}

