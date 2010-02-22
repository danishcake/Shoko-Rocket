#include "EditLevel.h"
#include <World.h>
#include <Walker.h>
#include <boost/foreach.hpp>
#include "Animation.h"
#include "GridTextureCreator.h"
using std::string;
using std::vector;
using boost::shared_ptr;

EditLevel::EditLevel(Vector2i _level_size, Vector2f _grid_size)
: BaseLevel(_grid_size)
{
	world_ = shared_ptr<World>(new World());
	world_->SetSize(_level_size);
	world_->SetArrowStockUnlimited();
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	edit_mode_ = EditMode::EditWalls;
}

EditLevel::EditLevel(std::string _filename, Vector2f _grid_size) 
: BaseLevel(_grid_size)
{
	world_ = shared_ptr<World>(new World(_filename));
	world_->LoadSolution();
	world_->SetArrowStockUnlimited();
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	edit_mode_ = EditMode::EditWalls;
}

EditLevel::~EditLevel(void)
{
}

void EditLevel::Tick(float _time, Input _input)
{
	timer_ += _time;
	_input.position += scroll_offset_;

	switch(edit_mode_)
	{
	case EditMode::EditArrows:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleNorthArrow(_input.position);
				validated_ = false;
				break;
			case Action::PlaceWestArrow:
				world_->ToggleWestArrow(_input.position);
				validated_ = false;
				break;
			case Action::PlaceEastArrow:
				world_->ToggleEastArrow(_input.position);
				validated_ = false;
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleSouthArrow(_input.position);
				validated_ = false;
				break;
			case Action::Cancel:
				world_->ClearArrows();
				validated_ = false;
				break;
			case Action::ClearSquare:
				world_->ClearArrow(_input.position);
				validated_ = false;
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				
				break;
			}
		}
		break;
	case EditMode::EditWalls:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleWall(_input.position, Direction::North);
				delete grid_animation_;
				grid_animation_ = CreateGridTexture(world_, grid_size_);
				validated_ = false;
				break;
			case Action::PlaceWestArrow:
				world_->ToggleWall(_input.position, Direction::West);
				delete grid_animation_;
				grid_animation_ = CreateGridTexture(world_, grid_size_);
				validated_ = false;
				break;
			case Action::PlaceEastArrow:
				world_->ToggleWall(_input.position, Direction::East);
				delete grid_animation_;
				grid_animation_ = CreateGridTexture(world_, grid_size_);
				validated_ = false;
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleWall(_input.position, Direction::South);
				delete grid_animation_;
				grid_animation_ = CreateGridTexture(world_, grid_size_);
				validated_ = false;
				break;
			case Action::ClearSquare:
				world_->SetGridSquare(_input.position, GridSquare(false, false, false, false));
				delete grid_animation_;
				grid_animation_ = CreateGridTexture(world_, grid_size_);
				validated_ = false;
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				break;
			}
		}
		break;
	case EditMode::EditCats:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleCat(_input.position, Direction::North);
				validated_ = false;
				break;
			case Action::PlaceWestArrow:
				world_->ToggleCat(_input.position, Direction::West);
				validated_ = false;
				break;
			case Action::PlaceEastArrow:
				world_->ToggleCat(_input.position, Direction::East);
				validated_ = false;
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleCat(_input.position, Direction::South);
				validated_ = false;
				break;
			case Action::Cancel:
				break;
			case Action::ClearSquare:
				//TODO
				validated_ = false;
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				break;
			}
		}
		break;
	case EditMode::EditMice:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleMouse(_input.position, Direction::North);
				validated_ = false;
				break;
			case Action::PlaceWestArrow:
				world_->ToggleMouse(_input.position, Direction::West);
				validated_ = false;
				break;
			case Action::PlaceEastArrow:
				world_->ToggleMouse(_input.position, Direction::East);
				validated_ = false;
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleMouse(_input.position, Direction::South);
				validated_ = false;
				break;
			case Action::Cancel:
				break;
			case Action::ClearSquare:
				//TODO
				validated_ = false;
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				break;
			}
		}
		break;
	case EditMode::EditHoles:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleHole(_input.position);
				validated_ = false;
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleHole(_input.position);
				break;
			case Action::PlaceEastArrow:
				world_->ToggleHole(_input.position);
				break;
			case Action::PlaceWestArrow:
				world_->ToggleHole(_input.position);
				break;
			case Action::Cancel:
				break;
			case Action::ClearSquare:
				world_->SetSquareType(_input.position, SquareType::Empty);
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				break;
			}
		}
		break;
	case EditMode::EditRockets:
		if(_input.position.x < world_->GetSize().x &&
		   _input.position.y < world_->GetSize().y)
		{
			switch(_input.action)
			{
			case Action::PlaceNorthArrow:
				world_->ToggleRocket(_input.position);
				break;
			case Action::PlaceSouthArrow:
				world_->ToggleRocket(_input.position);
				break;
			case Action::PlaceEastArrow:
				world_->ToggleRocket(_input.position);
				break;
			case Action::PlaceWestArrow:
				world_->ToggleRocket(_input.position);
			case Action::Cancel:
				break;
			case Action::ClearSquare:
				world_->SetSquareType(_input.position, SquareType::Empty);
				break;
			case Action::Start:
				edit_mode_restore_ = edit_mode_;
				edit_mode_ = EditMode::Running;
				break;
			}
		}
		break;

	case EditMode::RunningFast:
		_time *= 3;
		//Fall through
	case EditMode::Running:
		{
		WorldState::Enum result = world_->Tick(_time * 5.0f);
		switch(_input.action)
		{
		case Action::Cancel:
			edit_mode_ = edit_mode_restore_;
			world_->Reset();
			break;
		case Action::Start:
			edit_mode_ = EditMode::RunningFast;
			break;
		}
		if(result == WorldState::Defeat)
		{
			edit_mode_ = EditMode::Invalidated;
			timer_ = 0;
			validated_ = false;
		}

		if(result == WorldState::Victory)
		{
			edit_mode_ = EditMode::Validated;
			timer_ = 0;
			validated_ = true;
		}
		}
		break;


	case EditMode::Invalidated:
		//Fall through
	case EditMode::Validated:
		world_->Tick(_time * 5.0f);
		if(timer_ > 1.0f)
		{
			edit_mode_ = edit_mode_restore_;
			world_->Reset();
		}
		break;
	}

	BaseLevel::Tick(_time, _input);
}

void EditLevel::SetEditMode(EditMode::Enum _edit_mode)
{
	switch(edit_mode_)
	{
	case EditMode::EditArrows:
	case EditMode::EditCats:
	case EditMode::EditHoles:
	case EditMode::EditMice:
	case EditMode::EditRockets:
	case EditMode::EditWalls:
		edit_mode_ = _edit_mode;
		break;
	default:
		edit_mode_restore_ = _edit_mode;
		break;
	}
}

void EditLevel::Save(std::string _name)
{
	world_->SaveAs(_name);
}