#include "StatusLevel.h"
#include "GridTextureCreator.h"

using std::string;
using std::vector;
using boost::shared_ptr;

StatusLevel::StatusLevel(Vector2i _grid_size) 
: BaseLevel(_grid_size)
{
	world_ = shared_ptr<World>(new World());
	world_->SetSize(Vector2i(3,3));
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	world_->AddMouse(Vector2i(0,0), Direction::South);
	world_->AddMouse(Vector2i(0,1), Direction::South);
	world_->AddMouse(Vector2i(0,2), Direction::East);
	world_->AddMouse(Vector2i(1,2), Direction::East);
	world_->AddMouse(Vector2i(2,2), Direction::North);
	world_->AddMouse(Vector2i(2,1), Direction::North);
	world_->AddMouse(Vector2i(2,0), Direction::West);
	world_->AddMouse(Vector2i(1,0), Direction::West);
	state_ = StatusState::Stopped;
}

StatusLevel::~StatusLevel(void)
{
}

void StatusLevel::SetState(StatusState::Enum _state)
{
	state_ = _state;
	if(state_ != ltv_state_)
	{
		switch(state_)
		{
		case StatusState::Success:
			world_->RescueAllMice();
			break;
		case StatusState::Fail:
			world_->KillAllMice();
			break;
		case StatusState::Stopped:
			world_->Reset();
			break;
		}
	}
	ltv_state_ = state_;
}

void StatusLevel::Tick(float _time, Input _input)
{
	switch(state_)
	{
	case StatusState::RunningFast:
		_time *= 3;
		//Fall through
	case StatusState::Running:
	case StatusState::Fail:
	case StatusState::Success:
		world_->Tick(_time * 5.0f);
		break;
	default:
		//Do nothing
		break;
	}
	BaseLevel::Tick(_time, _input);
}
