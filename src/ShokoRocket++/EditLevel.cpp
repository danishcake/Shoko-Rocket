#include "EditLevel.h"
#include <World.h>
#include <Walker.h>
#include <boost/foreach.hpp>
#include "StandardTextures.h"
#include "Animation.h"
#include "GridTextureCreator.h"
using std::string;
using std::vector;
using boost::shared_ptr;

EditLevel::EditLevel(Vector2i _level_size, Vector2f _grid_size)
{
	world_ = shared_ptr<World>(new World());
	world_->SetSize(_level_size);
	world_->SetArrowStockUnlimited();
	grid_size_ = _grid_size;
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	edit_mode_ = EditMode::EditWalls;
	timer_ = 0;
}

EditLevel::~EditLevel(void)
{
	delete grid_animation_;
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

	switch(_input.action)
	{
	case Action::ScrollWest:
		if(scroll_offset_.x > 0)
			scroll_offset_.x--;
		break;
	case Action::ScrollEast:
		if(world_->GetSize().x - 12 - scroll_offset_.x > 0)
			scroll_offset_.x++;
		break;
	case Action::ScrollNorth:
		if(scroll_offset_.y > 0)
			scroll_offset_.y--;
		break;
	case Action::ScrollSouth:
		if(world_->GetSize().y - 9 - scroll_offset_.y > 0)
			scroll_offset_.y++;
		break;
	}
}

vector<RenderItem> EditLevel::Draw()
{	
	const float below = -1;
	const float above = 1;
	vector<RenderItem> draw_list;
	draw_list.reserve(world_->GetMice().size() + world_->GetCats().size() + world_->GetArrows().size());

	vector<Direction::Enum> arrows = world_->GetArrows();
	float arrow_id = 1;

	int arrow_count[5] = {0, 0, 0, 0, 0};
	BOOST_FOREACH(Direction::Enum arrow, arrows)
	{
		arrow_count[arrow]++;
	}
	for(int arrow_dir = 0; arrow_dir < 5; arrow_dir++)
	{
		if(arrow_count[arrow_dir] >= 3)
		{
			int index = arrow_count[arrow_dir] - 3;
			if(arrow_count[arrow_dir] > 9)
				index = 7;
			RenderItem ri;
			ri.position_ = Vector2f(arrow_id, static_cast<float>(world_->GetSize().y) + 1);
			ri.frame_ = StandardTextures::arrow_sets[arrow_dir]->GetFrameByIndex(index);
			ri.depth = above;
			arrow_id += 2;
			draw_list.push_back(ri);
		} else
		{
			for(int i = 0; i < arrow_count[arrow_dir]; i++)
			{
				RenderItem ri;
				ri.position_ = Vector2f(arrow_id, static_cast<float>(world_->GetSize().y) + 1);
				ri.frame_ = StandardTextures::arrows[arrow_dir]->GetCurrentFrame();
				ri.depth = above;
				arrow_id++;
				draw_list.push_back(ri);
			}
		}
	}

	vector<Vector2f> rings = world_->GetProblemPoints();
	BOOST_FOREACH(Vector2f point, rings)
	{
		RenderItem ri;
		ri.position_ = point;
		ri.frame_ = StandardTextures::ring_animation->GetCurrentFrame();
		ri.depth = above;
		draw_list.push_back(ri);
	}

	Walker* p_walker = NULL;
	BOOST_FOREACH(p_walker, world_->GetMice())
	{
		RenderItem ri;
		ri.position_ = p_walker->GetPosition();
		ri.frame_ = StandardTextures::mouse_animations[p_walker->GetDirection()]->GetCurrentFrame();
		ri.depth = 0;
		draw_list.push_back(ri);
	}
	BOOST_FOREACH(p_walker, world_->GetCats())
	{
		RenderItem ri;
		ri.position_ = p_walker->GetPosition();
		ri.frame_ = StandardTextures::cat_animations[p_walker->GetDirection()]->GetCurrentFrame();
		ri.depth = 0;
		draw_list.push_back(ri);
	}

	//Draw dead mice
	BOOST_FOREACH(p_walker, world_->GetDeadMice())
	{	
		if(p_walker->GetDeathTime() < 15 && p_walker->GetWalkerState() == WalkerState::Killed)
		{
			RenderItem ri;
			ri.position_ = p_walker->GetPosition();
			ri.position_.y -= (p_walker->GetDeathTime() / 10);
			ri.position_.x += 0.2f * sinf((p_walker->GetDeathTime()));
			ri.frame_ = StandardTextures::mouse_death->GetCurrentFrame();
			ri.depth = above;
			draw_list.push_back(ri);
		}
		if(p_walker->GetDeathTime() < 3 && p_walker->GetWalkerState() == WalkerState::Rescued)
		{
			RenderItem ri;
			ri.position_ = p_walker->GetPosition();
			ri.frame_ = StandardTextures::mouse_rescue->GetFrame(p_walker->GetDeathTime() / 3.0f);
			ri.depth = above;
			draw_list.push_back(ri);
		}
	}
	//Draw dead cats
	BOOST_FOREACH(p_walker, world_->GetDeadCats())
	{	
		if(p_walker->GetDeathTime() < 4 && p_walker->GetWalkerState() == WalkerState::Killed)
		{
			RenderItem ri;
			ri.position_ = p_walker->GetPosition();
			ri.frame_ = StandardTextures::cat_death->GetFrame(p_walker->GetDeathTime() / 4.0f);
			ri.depth = above;
			draw_list.push_back(ri);
		}
	}

	
	for(int x = 0; x < world_->GetSize().x; x++)
	{
		for(int y = 0; y < world_->GetSize().y; y++)
		{
			SquareType::Enum square_type = world_->GetSquareType(Vector2i(x, y));
			switch(square_type)
			{
			case SquareType::Rocket:
				{
					RenderItem ri;
					ri.position_ = Vector2f((float)x, (float)y);
					ri.frame_ = StandardTextures::rocket_normal_animation->GetCurrentFrame();
					ri.depth = below;
					draw_list.push_back(ri);
				}
				break;
			case SquareType::Hole:
				{
					RenderItem ri;
					ri.position_ = Vector2f((float)x, (float)y);
					ri.frame_ = StandardTextures::hole_animation->GetCurrentFrame();
					ri.depth = below;
					draw_list.push_back(ri);
				}
				break;
			case SquareType::NorthArrow:
			case SquareType::SouthArrow:
			case SquareType::EastArrow:
			case SquareType::WestArrow:
				{
					RenderItem ri;
					ri.position_ = Vector2f((float)x, (float)y);
					ri.frame_ = StandardTextures::arrows[SquareType::GetDirection(square_type)]->GetCurrentFrame();
					ri.depth = below;
					draw_list.push_back(ri);
				}
				break;
			case SquareType::HalfNorthArrow:
			case SquareType::HalfSouthArrow: 
			case SquareType::HalfEastArrow: 
			case SquareType::HalfWestArrow:
				{
					RenderItem ri;
					ri.position_ = Vector2f((float)x, (float)y);
					ri.frame_ = StandardTextures::half_arrows[SquareType::GetDirection(square_type)]->GetCurrentFrame();
					ri.depth = below;
					draw_list.push_back(ri);
				}
				break;
			}
		}
	}

	RenderItem grid;
	grid.position_ = Vector2f(0, 0);
	grid.depth = 2 * below;
	grid.frame_ = grid_animation_->GetCurrentFrame();
	draw_list.push_back(grid);

	BOOST_FOREACH(RenderItem& render_item, draw_list)
	{
		render_item.position_ -= scroll_offset_;
	}

	RenderItem arrow_area;
	arrow_area.position_ = Vector2f(0, 9 + 0.5f);
	arrow_area.frame_ = StandardTextures::arrows_area->GetCurrentFrame();
	arrow_area.depth = 2 * below;
	draw_list.push_back(arrow_area);

	BOOST_FOREACH(RenderItem& render_item, draw_list)
	{
		render_item.position_ *= grid_size_;
		render_item.depth *= grid_size_.y;
	}

	return draw_list;
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