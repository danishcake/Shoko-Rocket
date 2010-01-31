#include "GridSquare.h"

GridSquare::GridSquare(void)
{
	north_ = false;
	south_ = false;
	east_ = false;
	west_ = false;
}

GridSquare::GridSquare(bool _north, bool _south, bool _west, bool _east)
{
	north_ = _north;
	south_ = _south;
	east_ = _east;
	west_ = _west;
}

bool GridSquare::GetDirection(Direction::Enum _direction)
{
	switch(_direction)
	{
		case Direction::North:
			return north_;
			break;
		case Direction::South:
			return south_;
			break;
		case Direction::East:
			return east_;
			break;
		case Direction::West:
			return west_;
			break;
		default:
			//Should never reach here
			return false;
			//TODO assert
			break;
	}
}

void GridSquare::ToggleDirection(Direction::Enum _direction)
{
	switch(_direction)
	{
		case Direction::North:
			north_ ^= true;
			break;
		case Direction::South:
			south_ ^= true;
			break;
		case Direction::East:
			east_ ^= true;
			break;
		case Direction::West:
			west_ ^= true;
			break;
		default:
			//Should never reach here
			break;
	}
}

Direction::Enum GridSquare::GetTurnDirection(const Direction::Enum _current_direction)
{
	if(!GetDirection(_current_direction))
		return _current_direction;
	else if(GetDirection(_current_direction) && !GetDirection(Direction::TurnRight(_current_direction)))
		return Direction::TurnRight(_current_direction);
	else if(GetDirection(_current_direction) && 
	   GetDirection(Direction::TurnRight(_current_direction)) &&
	   !GetDirection(Direction::TurnLeft(_current_direction)))
		return Direction::TurnLeft(_current_direction);
	else if(!GetDirection(Direction::TurnAround(_current_direction)))
		return Direction::TurnAround(_current_direction);
	else
		return Direction::Stopped;
}