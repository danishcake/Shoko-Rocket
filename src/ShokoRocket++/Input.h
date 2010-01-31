#pragma once
#include <vmath.h>

namespace Action
{
	enum Enum
	{
		None, 
		PlaceWestArrow, PlaceEastArrow, PlaceNorthArrow, PlaceSouthArrow, 
		Cancel, ClearSquare, Start, LoadSolution, 
		ScrollNorth, ScrollSouth, ScrollEast, ScrollWest
	};
}

struct Input
{
	Vector2i position;
	Action::Enum action;
	Input()
	{
		action = Action::None;
		position = Vector2i(0, 0);
	}
};
