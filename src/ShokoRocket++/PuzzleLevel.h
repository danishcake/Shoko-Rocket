#pragma once
#include "BaseLevel.h"

namespace PuzzleMode
{
	enum Enum
	{
		Puzzle, Running, RunningFast, Defeat, Victory
	};
}

class PuzzleLevel : public BaseLevel
{
protected:
	PuzzleMode::Enum puzzle_state_;
public:
	PuzzleLevel(std::string _name, Vector2f _grid_size);
	virtual ~PuzzleLevel(void);

	//virtual std::vector<RenderItem> Draw();
	virtual void Tick(float _time, Input _input);
	PuzzleMode::Enum GetPuzzleState(){return puzzle_state_;}
};
