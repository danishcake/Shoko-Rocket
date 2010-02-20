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
protected:
	StatusState::Enum state_;
	StatusState::Enum ltv_state_;
public:
	StatusLevel(Vector2i _grid_size);
	virtual ~StatusLevel(void);
	void SetState(StatusState::Enum _state);
	virtual void Tick(float _time, Input _input);
};
