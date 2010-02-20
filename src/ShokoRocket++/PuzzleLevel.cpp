#include "PuzzleLevel.h"
#include <World.h>
#include <Walker.h>
#include <boost/foreach.hpp>
#include "GridTextureCreator.h"

using std::string;
using std::vector;
using boost::shared_ptr;

PuzzleLevel::PuzzleLevel(string _name, Vector2f _grid_size) 
	: BaseLevel(_grid_size)
{
	world_ = shared_ptr<World>(new World(_name));
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	puzzle_state_ = PuzzleMode::Puzzle;
	
}

PuzzleLevel::~PuzzleLevel(void)
{
}

void PuzzleLevel::Tick(float _time, Input _input)
{
	timer_ += _time;
	switch(puzzle_state_)
	{
	case PuzzleMode::Puzzle:
		switch(_input.action)
		{
		case Action::PlaceNorthArrow:
			world_->ToggleNorthArrow(_input.position);
			break;
		case Action::PlaceWestArrow:
			world_->ToggleWestArrow(_input.position);
			break;
		case Action::PlaceEastArrow:
			world_->ToggleEastArrow(_input.position);
			break;
		case Action::PlaceSouthArrow:
			world_->ToggleSouthArrow(_input.position);
			break;
		case Action::Cancel:
			world_->ClearArrows();
			break;
		case Action::ClearSquare:
			world_->ClearArrow(_input.position);
			break;
		case Action::Start:
			puzzle_state_ = PuzzleMode::Running;
			break;
		case Action::LoadSolution:
			world_->LoadSolution();
			break;

		}
		break;



	case PuzzleMode::RunningFast:
		_time *= 3;
		//Fall through
	case PuzzleMode::Running:
		{
		WorldState::Enum result = world_->Tick(_time * 5.0f);
		switch(_input.action)
		{
		case Action::Cancel:
			puzzle_state_ = PuzzleMode::Puzzle;
			world_->Reset();
			break;
		case Action::Start:
			puzzle_state_ = PuzzleMode::RunningFast;
			break;
		}
		if(result == WorldState::Defeat)
		{
			puzzle_state_ = PuzzleMode::Defeat;
			timer_ = 0;
		}

		if(result == WorldState::Victory)
		{
			puzzle_state_ = PuzzleMode::Victory;
			timer_ = 0;
		}
		}
		break;


	case PuzzleMode::Victory:
		//Fall through
	case PuzzleMode::Defeat:
		world_->Tick(_time * 5.0f);
		if(timer_ > 2.5f)
		{
			puzzle_state_ = PuzzleMode::Puzzle;
			world_->Reset();
		}
		break;
	}
	BaseLevel::Tick(_time, _input);
}
