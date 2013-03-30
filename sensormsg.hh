#ifndef SENSORMSG_HH_
#define SENSORMSG_HH_

#include <string>

class SensorMessage
{
  public:
	SensorMessage(const std::string message);
	void printMessage() const;

  private:
	std::string message;
};

#endif
