#pragma once
#include "BaseLevel.h"

namespace StatusState
{
	enum Enum
	{
		Stopped, Running, RunningFast, Fail, Success
	};
}

class StatusLevel :
	public BaseLevel
{
public:
	StatusLevel(Vector2i _grid_size);
	virtual ~StatusLevel(void);
};
