#include "PuzzleLevel.h"
#include <World.h>
#include <Walker.h>
#include <boost/foreach.hpp>
#include "StandardTextures.h"
#include "Animation.h"
#include "GridTextureCreator.h"
using std::string;
using std::vector;
using boost::shared_ptr;

PuzzleLevel::PuzzleLevel(string _name, Vector2f _grid_size)
{
	world_ = shared_ptr<World>(new World(_name));
	grid_size_ = _grid_size;
	grid_animation_ = CreateGridTexture(world_, grid_size_);
	puzzle_state_ = PuzzleMode::Puzzle;
	timer_ = 0;
}

PuzzleLevel::~PuzzleLevel(void)
{
	delete grid_animation_;
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
		WorldState::Enum result = world_->Tick(_time * 5.0f);
		if(timer_ > 2.5f)
		{
			puzzle_state_ = PuzzleMode::Puzzle;
			world_->Reset();
		}
		break;
	}
}

vector<RenderItem> PuzzleLevel::Draw()
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
			ri.position_ = Vector2i(arrow_id, world_->GetSize().y + 1);
			ri.frame_ = StandardTextures::arrow_sets[arrow_dir]->GetFrameByIndex(index);
			ri.depth = above;
			arrow_id += 2;
			draw_list.push_back(ri);
		} else
		{
			for(int i = 0; i < arrow_count[arrow_dir]; i++)
			{
				RenderItem ri;
				ri.position_ = Vector2i(arrow_id, world_->GetSize().y + 1);
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


	RenderItem arrow_area;
	arrow_area.position_ = Vector2f(0, world_->GetSize().y + 0.5f);
	arrow_area.frame_ = StandardTextures::arrows_area->GetCurrentFrame();
	arrow_area.depth = 2 * below;
	draw_list.push_back(arrow_area);


	RenderItem grid;
	grid.position_ = Vector2f(0, 0);
	grid.depth = 2 * below;
	grid.frame_ = grid_animation_->GetCurrentFrame();
	draw_list.push_back(grid);

	BOOST_FOREACH(RenderItem& render_item, draw_list)
	{
		render_item.position_ *= grid_size_;
		render_item.depth *= grid_size_.y;
	}

	return draw_list;
}
