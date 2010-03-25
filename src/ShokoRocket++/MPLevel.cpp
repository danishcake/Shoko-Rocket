#include "MPLevel.h"
#include <Animation.h>
#include <Walker.h>
#include "StandardTextures.h"
#include <boost/foreach.hpp>
#include "GridTextureCreator.h"

MPLevel::MPLevel(Vector2f _grid_size, MPWorld* _world)
{
	grid_size_ = _grid_size;
	world_ = boost::shared_ptr<MPWorld>(_world);
	grid_animation_ = CreateGridTexture(world_, grid_size_);
}

MPLevel::~MPLevel(void)
{
}

void MPLevel::HandleOpcodes(std::vector<Opcodes::ServerOpcode*> _opcodes)
{
	for(std::vector<Opcodes::ServerOpcode*>::iterator opcode = _opcodes.begin(); opcode != _opcodes.end(); opcode++)
	{
		switch((*opcode)->opcode_)
		{
		case Opcodes::DriveCursor::OPCODE:
			break;
		case Opcodes::WalkerSpawn::OPCODE:
			{
				//TODO better out of range handling via defaults
				Opcodes::WalkerSpawn* walker_spawn = (Opcodes::WalkerSpawn*)*opcode;
				
				Direction::Enum direction;
				Vector2f position = walker_spawn->position_;
				switch(walker_spawn->direction_)
				{
				case Opcodes::WalkerSpawn::DIRECTION_NORTH:
					direction = Direction::North;
					break;
				case Opcodes::WalkerSpawn::DIRECTION_SOUTH:
					direction = Direction::South;
					break;
				case Opcodes::WalkerSpawn::DIRECTION_EAST:
					direction = Direction::East;
					break;
				case Opcodes::WalkerSpawn::DIRECTION_WEST:
				default:
					direction = Direction::West;
					break;
				}
				
				switch(walker_spawn->walker_type_)
				{
				case Opcodes::WalkerSpawn::WALKER_CAT:
					world_->CreateCat(walker_spawn->uid_, position, direction, walker_spawn->time_);
					break;
				case Opcodes::WalkerSpawn::WALKER_MOUSE:
				default:
					world_->CreateMouse(walker_spawn->uid_, position, direction, walker_spawn->time_);
					break;
				}
			}
			break;
		case Opcodes::KillWalker::OPCODE:
			{
				Opcodes::KillWalker* kill_walker_opcode = (Opcodes::KillWalker*)*opcode;
				world_->RemoveWalker(kill_walker_opcode->uid_, kill_walker_opcode->death_, kill_walker_opcode->position_, kill_walker_opcode->time_);
			}
			break;
		case Opcodes::WalkerUpdate::OPCODE:
			{
				Opcodes::WalkerUpdate* walker_update_opcode = (Opcodes::WalkerUpdate*)*opcode;
				Direction::Enum direction;
				switch(walker_update_opcode->direction_)
				{
				case Opcodes::WalkerUpdate::DIRECTION_NORTH:
					direction = Direction::North;
					break;
				case Opcodes::WalkerUpdate::DIRECTION_SOUTH:
					direction = Direction::South;
					break;
				case Opcodes::WalkerUpdate::DIRECTION_EAST:
					direction = Direction::East;
					break;
				case Opcodes::WalkerUpdate::DIRECTION_WEST:
				default:
					direction = Direction::West;
					break;
				}


				world_->UpdateWalker(walker_update_opcode->uid_, walker_update_opcode->position_, direction, walker_update_opcode->time_);
			}
			break;
		case Opcodes::ArrowSpawn::OPCODE:
			{
				Opcodes::ArrowSpawn* arrow_spawn = (Opcodes::ArrowSpawn*)*opcode;
				Vector2i position = arrow_spawn->position_;
				int player_id = (int)arrow_spawn->player_;
				PlayerArrowLevel::Enum arrow_level;
				
				Direction::Enum direction;

				switch(arrow_spawn->direction_)
				{
				case Opcodes::ArrowSpawn::DIRECTION_NORTH:
					direction = Direction::North;
					break;
				case Opcodes::ArrowSpawn::DIRECTION_SOUTH:
					direction = Direction::South;
					break;
				case Opcodes::ArrowSpawn::DIRECTION_EAST:
					direction = Direction::East;
					break;
				case Opcodes::ArrowSpawn::DIRECTION_WEST:
				default:
					direction = Direction::West;
					break;
				}

				switch(arrow_spawn->arrow_state_)
				{
				case Opcodes::ArrowSpawn::ARROW_FULL:
					arrow_level = PlayerArrowLevel::FullArrow;
					break;
				case Opcodes::ArrowSpawn::ARROW_HALF:
					arrow_level = PlayerArrowLevel::HalfArrow;
					break;
				case Opcodes::ArrowSpawn::ARROW_CLEAR:
				default:
					arrow_level = PlayerArrowLevel::Clear;
					break;
				}

				world_->SetPlayerArrow(position, direction, player_id, arrow_level);
			}
			break;
		default:
			break;
		}
	}
}
void MPLevel::Tick(float _time)
{
	timer_ += _time;
	WorldState::Enum result = world_->Tick(_time * 5.0f);
}

std::vector<RenderItem> MPLevel::Draw()
{
	const float below = -1;
	const float above = 1;
	vector<RenderItem> draw_list;
	draw_list.reserve(world_->GetMice().size() + world_->GetCats().size());

	//Draw mice and cats
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
	
	//Draw player arrows, spawners, holes and rockets
	std::vector<PlayerArrow> player_arrows = world_->GetPlayerArrows();
	for(std::vector<PlayerArrow>::iterator it = player_arrows.begin(); it != player_arrows.end(); ++it)
	{
		if(it->player_id >= 0 && it->player_id < 8)
		{
			RenderItem ri;
			ri.position_ = it->position;
			ri.depth = below;
			if(it->halved)
				ri.frame_ = StandardTextures::mp_half_arrows[it->player_id][it->direction]->GetCurrentFrame();
			else
				ri.frame_ = StandardTextures::mp_arrows[it->player_id][it->direction]->GetCurrentFrame();
			draw_list.push_back(ri);
		}
	}

	std::vector<PlayerRocket> player_rockets = world_->GetPlayerRockets();
	for(std::vector<PlayerRocket>::iterator it = player_rockets.begin(); it != player_rockets.end(); ++it)
	{
		if(it->player_id >= 0 && it->player_id < 8)
		{
			RenderItem ri;
			ri.position_ = it->position;
			ri.depth = below;
			ri.frame_ = StandardTextures::mp_rockets[it->player_id]->GetCurrentFrame();
			draw_list.push_back(ri);
		}
	}

	std::vector<Spawner> spawners = world_->GetSpawners();
	for(std::vector<Spawner>::iterator it = spawners.begin(); it != spawners.end(); ++it)
	{
		RenderItem ri;
		ri.position_ = it->position;
		ri.depth = below;
		ri.frame_ = StandardTextures::mp_spawner->GetCurrentFrame();
		draw_list.push_back(ri);
	}

	for(int x = 0; x < world_->GetSize().x; x++)
	{
		for(int y = 0; y < world_->GetSize().y; y++)
		{
			SquareType::Enum square_type = world_->GetSquareType(Vector2i(x, y));
			switch(square_type)
			{
			case SquareType::Hole:
				{
					RenderItem ri;
					ri.position_ = Vector2f((float)x, (float)y);
					ri.frame_ = StandardTextures::hole_animation->GetCurrentFrame();
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