#pragma once
#include "Direction.h"

class GridSquare
{
private:
	bool north_;
	bool south_;
	bool west_;
	bool east_;

public:
	GridSquare(void);
	GridSquare(bool _north, bool _south, bool _west, bool _east);

	bool GetNorth(){return north_;}
	void SetNorth(bool _north){north_ = _north;}
	void SetNorth(){north_ = true;}
	void ClearNorth(){north_ = false;}

	bool GetSouth(){return south_;}
	void SetSouth(bool _south){south_ = _south;}
	void SetSouth(){south_ = true;}
	void ClearSouth(){south_= false;}

	bool GetWest(){return west_;}
	void SetWest(bool _west){west_ = _west;}
	void SetWest(){west_ = true;}
	void ClearWest(){west_ = false;}

	bool GetEast(){return east_;}
	void SetEast(bool _east){east_ = _east;}
	void SetEast(){east_ = true;}
	void ClearEast(){east_ = false;}

	bool GetDirection(Direction::Enum _direction);
	void ToggleDirection(Direction::Enum _direction);

	void Clear(){north_ = false; south_ = false; east_ = false; west_ = false;}

	Direction::Enum GetTurnDirection(const Direction::Enum _current_direction);
};
