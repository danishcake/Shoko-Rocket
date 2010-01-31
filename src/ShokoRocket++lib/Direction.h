#pragma once
#include <string>

/*
Coordinates correspond to: North = y--
                           South = y++
						   East  = x--
						   West  = x++
*/

namespace Direction
{
	enum Enum
	{
		North, South, East, West, Stopped
	};

	Enum TurnRight(Enum _direction);
	Enum TurnLeft(Enum _direction);
	Enum TurnAround(Enum _direction);
	Enum FromString(std::string _direction_string);
	std::string ToString(Enum _direction);
}
