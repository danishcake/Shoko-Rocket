#include "BaseLevel.h"
#include <boost/foreach.hpp>
#include <Walker.h>
#include "StandardTextures.h"
#include "Animation.h"
using std::string;
using std::vector;
using boost::shared_ptr;

BaseLevel::BaseLevel(Vector2f _grid_size)
{
	grid_size_ = _grid_size;
	timer_ = 0;
	grid_animation_ = NULL;
}
BaseLevel::~BaseLevel()
{
	delete grid_animation_;
}

int BaseLevel::ComputeArrowHash()
{
	int arrow_hash = 0;
	std::vector<Direction::Enum> arrows = world_->GetArrows();

	int arrow_count[5] = {0, 0, 0, 0, 0};
	BOOST_FOREACH(Direction::Enum arrow, arrows)
	{
		arrow_count[arrow]++;
	}
	for(int i = 0; i < 4; i++)
	{
		arrow_hash += (arrow_count[i] << (8*i));
	}

	return arrow_hash;
}

vector<RenderItem> BaseLevel::Draw()
{
	const float below = -1;
	const float above = 1;
	vector<RenderItem> draw_list;
	draw_list.reserve(world_->GetMice().size() + world_->GetCats().size());

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
		render_item.position_ *= grid_size_;
		render_item.depth *= grid_size_.y;
	}

	return draw_list;
}

void BaseLevel::Tick(float _time, Input _input)
{
	switch(_input.action)
	{
	case Action::ScrollWest:
		if(scroll_offset_.x + scroll_remaining_.x > 0.1)
		{
			scroll_remaining_.x--;
		}
		break;
	case Action::ScrollEast:
		if(scroll_offset_.x + scroll_remaining_.x < scroll_limit_.x - 0.1)
		{
			scroll_remaining_.x++;
		}
		break;
	case Action::ScrollNorth:
		if(scroll_offset_.y + scroll_remaining_.y > 0.1)
		{
			scroll_remaining_.y--;
		}
		break;
	case Action::ScrollSouth:
		if(scroll_offset_.y + scroll_remaining_.y < scroll_limit_.y - 0.1)
		{
			scroll_remaining_.y++;
		}
		break;
	}
	const float scroll_speed = 10;
	const float scroll_smoothing = 10;
	const float min_scroll_speed = 0.2;
	if(abs(scroll_remaining_.x) > 0)
	{
		float delta = ceil(abs(scroll_remaining_.x * scroll_smoothing)) * (scroll_remaining_.x >= 0 ? 1 : -1) / scroll_smoothing;
		if(abs(delta) < min_scroll_speed)
			delta = delta * min_scroll_speed / abs(delta);
		delta *= _time * scroll_speed;
		if(abs(delta) < abs(scroll_remaining_.x))
		{
			scroll_offset_.x += delta;
			scroll_remaining_.x -= delta;
		}
		else
		{
			scroll_offset_.x = floor(scroll_offset_.x + 0.5f);
			scroll_remaining_.x = 0;
		}
	}
	if(abs(scroll_remaining_.y) > 0)
	{
		float delta = ceil(abs(scroll_remaining_.y * scroll_smoothing)) * (scroll_remaining_.y >= 0 ? 1 : -1) / scroll_smoothing;
		if(abs(delta) < min_scroll_speed)
			delta = delta * min_scroll_speed / abs(delta);
		delta *= _time * scroll_speed;
		if(abs(delta) < abs(scroll_remaining_.y))
		{
			scroll_offset_.y += delta;
			scroll_remaining_.y -= delta;
		}
		else
		{
			scroll_offset_.y = floor(scroll_offset_.y + 0.5f);
			scroll_remaining_.y = 0;
		}
	}
}