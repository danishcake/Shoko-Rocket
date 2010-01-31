#include "Direction.h"
namespace Direction
{
	Enum FromString(std::string _direction_string)
	{
		if(_direction_string == "North")
			return North;
		if(_direction_string == "West")
			return West;
		if(_direction_string == "East")
			return East;
		if(_direction_string == "South")
			return South;
		return Stopped;
	}

	Enum TurnRight(Enum _direction)
	{
		switch(_direction)
		{
			case North:
				return East;
				break;
			case South:
				return West;
				break;
			case East:
				return South;
				break;
			case West:
				return North;
				break;
			default:
				//TODO assert(false);
				return Stopped;
				break;
		}
	}

	Enum TurnLeft(Enum _direction)
	{
		switch(_direction)
		{
			case North:
				return West;
				break;
			case South:
				return East;
				break;
			case East:
				return North;
				break;
			case West:
				return South;
				break;
			default:
				//TODO assert(false);
				return Stopped;
				break;
		}
	}

	Enum TurnAround(Enum _direction)
	{
		switch(_direction)
		{
			case North:
				return South;
				break;
			case South:
				return North;
				break;
			case East:
				return West;
				break;
			case West:
				return East;
				break;
			default:
				//TODO assert(false);
				return Stopped;
				break;
		}
	}

	std::string ToString(Enum _direction)
	{
		switch(_direction)
		{
			case North:
				return "North";
				break;
			case South:
				return "South";
				break;
			case East:
				return "East";
				break;
			case West:
				return "West";
				break;
			default:
				//TODO assert(false);
				return "Stopped";
				break;
		}
	}
}
