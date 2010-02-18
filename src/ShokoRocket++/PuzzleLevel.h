#pragma once
struct SDL_Surface;
#include <World.h>
#include "RenderItem.h"
#include "Input.h"
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

class Animation;

namespace PuzzleMode
{
	enum Enum
	{
		Puzzle, Running, RunningFast, Defeat, Victory
	};
}

class PuzzleLevel
{
protected:
	boost::shared_ptr<World> world_;
	PuzzleMode::Enum puzzle_state_;
	float timer_;
	Vector2f grid_size_;
	Vector2f scroll_remaining_;
	Vector2f scroll_offset_;
	Vector2i scroll_limit_;
public:
	PuzzleLevel(std::string _name, Vector2f _grid_size);
	~PuzzleLevel(void);

	std::vector<RenderItem> Draw();
	void Tick(float _time, Input _input);
	PuzzleMode::Enum GetPuzzleState(){return puzzle_state_;}
	Animation* grid_animation_;
	std::string GetFilename(){return world_->GetFilename();}
	Vector2i GetLevelSize(){return world_->GetSize();}
	std::vector<Direction::Enum> GetArrows(){return world_->GetArrows();}
	int PuzzleLevel::ComputeArrowHash();
	void SetScrollLimit(Vector2i _scroll_limit){scroll_limit_ = _scroll_limit;}
};
