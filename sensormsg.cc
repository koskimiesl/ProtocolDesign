#include "sensormsg.hh"
#include <iostream>

SensorMessage::SensorMessage(const std::string message):message(message){ }

void SensorMessage::printMessage() const
{
	std::cout << message << std::endl;
}
