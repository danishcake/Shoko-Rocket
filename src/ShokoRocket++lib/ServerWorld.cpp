#include "ServerWorld.h"
#include <algorithm>
#include "Walker.h"
#include "Logger.h"
#include "Random.h"

ServerWorld::ServerWorld(int _player_count) : MPWorld()
{
	arrow_limit_ = 3;
	player_count_ = _player_count;
	players_ready_ = 0;
	if(player_count_ == 0) //Unit tests are run with zero players
		server_game_state_ = ServerGameState::Running;
	else
		server_game_state_ = ServerGameState::WaitingForClients;

	spawn_time_ = Random::RandomRange(0.1, 0.4);
	time_run_ = 0;
	walker_id_cnt_ = 0;
}

ServerWorld::ServerWorld(string _filename, int _player_count) : MPWorld(_filename)
{
	arrow_limit_ = 3;
	player_count_ = _player_count;
	players_ready_ = 0;
	if(player_count_ == 0)
		server_game_state_ = ServerGameState::Running;
	else
		server_game_state_ = ServerGameState::WaitingForClients;
	spawn_time_ = Random::RandomRange(0.1, 0.4);
	time_run_ = 0;
	walker_id_cnt_ = 0;
}

WorldState::Enum ServerWorld::Tick(float _dt)
{
	//TODO perhaps split large DT into smaller periods?
	const float max_dt = 0.1f;
	int full_iterations = floor(_dt/max_dt);
	float last_iteration = _dt - full_iterations * max_dt;
	if(server_game_state_ == ServerGameState::Running)
	{
		for(int i = 0; i < full_iterations + 1; i++)
		{
			float dt;
			if(i == full_iterations)
				dt = last_iteration;
			else
				dt = max_dt;
			if(dt <= 0)
				continue;
			time_run_ += dt;
			if(state_ == WorldState::OK)
			{
				//Spawn
				if(mice_.size() < 64 && time_run_ >= spawn_time_) //TODO make flexible
				{
					SpawnWalkers();
					spawn_time_ = time_run_ + Random::RandomRange(0.5, 1.5);
				}

				//Walk forward
				for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
				{
					(*it)->Advance(dt);
				}
				for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
				{
					(*it)->Advance(dt);
				}
				//Collide cats and mice
				for(vector<Walker*>::iterator m_it = mice_.begin(); m_it != mice_.end(); ++m_it)
				{
					for(vector<Walker*>::iterator c_it = cats_.begin(); c_it != cats_.end(); ++c_it)
					{
						if(GetShortestDistance((*m_it)->GetPosition(), (*c_it)->GetPosition(), size_) < 0.3333f)
						{
							if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), *m_it) == just_dead_mice_.end())
							{
								just_dead_mice_.push_back(*m_it);
								(*m_it)->Kill();
								GenerateWalkerDeath((*m_it)->GetID(), (*m_it)->GetPosition(), true);
							}
						}
					}
				}	
				//Remove cats and mice that have just died, from holes, rockets and cats etc
				for(vector<Walker*>::iterator it = just_dead_mice_.begin(); it != just_dead_mice_.end(); ++it)
				{
					mice_.erase(std::remove(mice_.begin(), mice_.end(), *it), mice_.end());
					dead_mice_.push_back(*it);
				}
				just_dead_mice_.clear();
				for(vector<Walker*>::iterator it = just_dead_cats_.begin(); it != just_dead_cats_.end(); ++it)
				{
					cats_.erase(std::remove(cats_.begin(), cats_.end(), *it), cats_.end());
					dead_cats_.push_back(*it);
				}
				just_dead_cats_.clear();
				//TODO scoring
			}
		}
		BaseWorld::Tick(_dt);

		//TODO cull totally dead mice
	}

	return state_;
}
void ServerWorld::WalkerReachNewSquare(Walker* _walker)
{
	//This method will be called multiple times, so it's important not to add an item to 'just_dead...' twice
	Vector2i position_grid = Vector2i(static_cast<int>(floor(_walker->GetPosition().x + 0.5f)), static_cast<int>(floor(_walker->GetPosition().y + 0.5f)));
	SquareType::Enum square_type = special_squares_[position_grid.x][position_grid.y];
	switch(_walker->GetWalkerType())
	{
	case WalkerType::Mouse:
		if(square_type == SquareType::Hole)
		{
			if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), _walker) == just_dead_mice_.end())
			{
				just_dead_mice_.push_back(_walker);
				_walker->Kill();
				GenerateWalkerDeath(_walker->GetID(), _walker->GetPosition(), true);
			}
		} 

		for(vector<PlayerRocket>::iterator it = player_rockets_.begin(); it != player_rockets_.end(); it++)
		{
			if((it->position - position_grid).lengthSq() < 0.1f)
			{
				if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), _walker) == just_dead_mice_.end())
				{
					just_dead_mice_.push_back(_walker);
					_walker->Rescue();
					GenerateWalkerDeath(_walker->GetID(), _walker->GetPosition(), false);
				}
			}
		}
		break;
	case WalkerType::Cat:
		if(square_type == SquareType::Hole)
		{
			if(std::find(just_dead_cats_.begin(), just_dead_cats_.end(), _walker) == just_dead_cats_.end())
			{
				just_dead_cats_.push_back(_walker);
				_walker->Kill();
				GenerateWalkerDeath(_walker->GetID(), _walker->GetPosition(), true);
			}
		}
		for(vector<PlayerRocket>::iterator it = player_rockets_.begin(); it != player_rockets_.end(); it++)
		{
			if((it->position - position_grid).lengthSq() < 0.1f)
			{
				if(std::find(just_dead_cats_.begin(), just_dead_cats_.end(), _walker) == just_dead_cats_.end())
				{
					just_dead_cats_.push_back(_walker);
					_walker->Kill();
					GenerateWalkerDeath(_walker->GetID(), _walker->GetPosition(), true);
				}
			}
		}
		break;
	}
	
	//TODO collide with player arrows
	bool arrow_needs_removing = false;
	vector<PlayerArrow>::iterator arrow_to_remove;
	for(vector<PlayerArrow>::iterator it = player_arrows_.begin(); it != player_arrows_.end(); ++it)
	{
		if((it->position - position_grid).lengthSq() < 0.1f)
		{
			//Cat should destroy the arrow
			if(_walker->GetWalkerType() == WalkerType::Cat && 
			   it->direction == Direction::TurnAround(_walker->GetDirection()))
			{
				if(it->halved)
				{
					arrow_needs_removing = true;
					arrow_to_remove = it;
					GenerateArrowOpcode(it->player_id, it->position, it->direction, PlayerArrowLevel::Clear);
				} else
				{
					it->halved = true;
					GenerateArrowOpcode(it->player_id, it->position, it->direction, PlayerArrowLevel::HalfArrow);
				}
			}
			_walker->EncounterArrow(it->direction);
			GenerateWalkerUpdate(_walker);
		}
	}
	if(arrow_needs_removing)
	{
		player_arrows_.erase(arrow_to_remove);
	}
}

bool ServerWorld::GetPlayerArrow(Vector2i _position, PlayerArrow& _arrow)
{
	for(vector<PlayerArrow>::iterator it = player_arrows_.begin(); it != player_arrows_.end(); ++it)
	{
		if(it->position == _position)
		{
			_arrow = *it;
			return true;
		}
			
	}
	return false;
}

void ServerWorld::HandleOpcodes(vector<vector<Opcodes::ClientOpcode*> > _opcodes)
{
	int player = 0;
	for(vector<vector<Opcodes::ClientOpcode*> >::iterator it = _opcodes.begin(); it != _opcodes.end(); ++it)
	{
		for(vector<Opcodes::ClientOpcode*>::iterator opcode_it = it->begin(); opcode_it != it->end(); ++opcode_it)
		{
			switch((*opcode_it)->opcode_)
			{
			case Opcodes::SendInput::OPCODE:
				HandleInputOpcode(player, (Opcodes::SendInput*)*opcode_it);
				break;
			case Opcodes::SetName::OPCODE:
				break;
			case Opcodes::UpdateCursor::OPCODE:
				break;
			case Opcodes::LoadComplete::OPCODE:
				players_ready_++;
				if(players_ready_ >= player_count_)
				{
					server_game_state_ = ServerGameState::Running;
				}
				break;
			}
			delete *opcode_it;
		}
		player++;
	}
}

void ServerWorld::GenerateWalkerSpawn(Walker* _walker)
{
	Opcodes::WalkerSpawn::WalkerType walker_type;
	Opcodes::WalkerSpawn::Direction direction;
	switch(_walker->GetDirection())
	{
	case Direction::East:
		direction = Opcodes::WalkerSpawn::DIRECTION_EAST;
		break;
	case Direction::West:
		direction = Opcodes::WalkerSpawn::DIRECTION_WEST;
		break;
	case Direction::North:
		direction = Opcodes::WalkerSpawn::DIRECTION_NORTH;
		break;
	case Direction::South:
		direction = Opcodes::WalkerSpawn::DIRECTION_SOUTH;
		break;
	default:
		assert(false);
		break;
	}
	switch(_walker->GetWalkerType())
	{
	case WalkerType::Mouse:
		walker_type = Opcodes::WalkerSpawn::WALKER_MOUSE;
		break;
	case WalkerType::Cat:
		walker_type = Opcodes::WalkerSpawn::WALKER_CAT;
		break;
	default:
		assert(false);
		break;
	}

	Opcodes::WalkerSpawn* opcode = new Opcodes::WalkerSpawn(walker_type, _walker->GetPosition(), direction, _walker->GetID());
	opcode->time_ = static_cast<unsigned int>(time_run_ * 1000);
	opcodes_to_clients_.push_back(opcode);
}
void ServerWorld::GenerateArrowOpcode(int _player_id, Vector2i _position, Direction::Enum _direction, PlayerArrowLevel::Enum _arrow_state)
{
	Opcodes::ArrowSpawn::Direction d = Opcodes::ArrowSpawn::DIRECTION_EAST;
	switch(_direction)
	{
	case Direction::East:
		d = Opcodes::ArrowSpawn::DIRECTION_EAST;
		break;
	case Direction::West:
		d = Opcodes::ArrowSpawn::DIRECTION_WEST;
		break;
	case Direction::North:
		d = Opcodes::ArrowSpawn::DIRECTION_NORTH;
		break;
	case Direction::South:
		d = Opcodes::ArrowSpawn::DIRECTION_SOUTH;
		break;
	}
	Opcodes::ArrowSpawn::ArrowState as = Opcodes::ArrowSpawn::ARROW_CLEAR;
	switch(_arrow_state)
	{
	case PlayerArrowLevel::FullArrow:
		as = Opcodes::ArrowSpawn::ARROW_FULL;
		break;
	case PlayerArrowLevel::HalfArrow:
		as = Opcodes::ArrowSpawn::ARROW_HALF;
		break;
	case PlayerArrowLevel::Clear:
		as = Opcodes::ArrowSpawn::ARROW_CLEAR;
		break;
	}
	Opcodes::ArrowSpawn* opcode = new Opcodes::ArrowSpawn(as, _position, d, _player_id);
	opcode->time_ = static_cast<unsigned int>(time_run_ * 1000);
	opcodes_to_clients_.push_back(opcode);
}

void ServerWorld::GenerateWalkerDeath(int _uid, Vector2f _position, bool _death)
{
	Opcodes::KillWalker* opcode = new Opcodes::KillWalker(_position, _uid, _death);
	opcode->time_ = static_cast<unsigned int>(time_run_ * 1000);
	opcodes_to_clients_.push_back(opcode);
}


void ServerWorld::GenerateWalkerUpdate(Walker* _walker)
{	
	Opcodes::WalkerUpdate::Direction direction;
	switch(_walker->GetDirection())
	{
	case Direction::East:
		direction = Opcodes::WalkerUpdate::DIRECTION_EAST;
		break;
	case Direction::West:
		direction = Opcodes::WalkerUpdate::DIRECTION_WEST;
		break;
	case Direction::North:
		direction = Opcodes::WalkerUpdate::DIRECTION_NORTH;
		break;
	case Direction::South:
		direction = Opcodes::WalkerUpdate::DIRECTION_SOUTH;
		break;
	default:
		assert(false);
		break;
	}

	Opcodes::WalkerUpdate* opcode = new Opcodes::WalkerUpdate(_walker->GetPosition(), direction, _walker->GetID());
	opcodes_to_clients_.push_back(opcode);
}
void ServerWorld::HandleInputOpcode(int _player_id, Opcodes::SendInput* _input)
{
	PlayerArrow arrow;
	bool arrow_present = GetPlayerArrow(_input->position_, arrow);

	//TODO check bounds of input
	//TODO generate server opcodes to inform clients of change

	//If arrow is one of mine then clear if same direction, rotate if different.
	//Otherwise no action, space taken
	if(server_game_state_ == ServerGameState::Running)
	{
		switch(_input->action_)
		{
		case Opcodes::SendInput::ACT_NORTH:
			if(arrow_present)
			{
				if(arrow.player_id == _player_id)
				{
					if(arrow.direction == Direction::North) //Clear
					{
						SetPlayerArrow(_input->position_, Direction::Stopped, -1, PlayerArrowLevel::Clear);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::Stopped, PlayerArrowLevel::Clear);
					}
					else //Replace
					{
						SetPlayerArrow(_input->position_, Direction::North, _player_id, PlayerArrowLevel::FullArrow);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::North, PlayerArrowLevel::FullArrow);
					}
				}
			} else if(CountArrows(_player_id) < arrow_limit_)
			{//New arrow
				SetPlayerArrow(_input->position_, Direction::North, _player_id, PlayerArrowLevel::FullArrow);
				GenerateArrowOpcode(_player_id, _input->position_, Direction::North, PlayerArrowLevel::FullArrow);
			}
			break;
		case Opcodes::SendInput::ACT_SOUTH:
			if(arrow_present)
			{
				if(arrow.player_id == _player_id)
				{
					if(arrow.direction == Direction::South) //Clear
					{
						SetPlayerArrow(_input->position_, Direction::Stopped, -1, PlayerArrowLevel::Clear);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::Stopped, PlayerArrowLevel::Clear);
					}
					else //Replace
					{
						SetPlayerArrow(_input->position_, Direction::South, _player_id, PlayerArrowLevel::FullArrow);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::South, PlayerArrowLevel::FullArrow);
					}
				}
			} else if(CountArrows(_player_id) < arrow_limit_)
			{//New arrow
				SetPlayerArrow(_input->position_, Direction::South, _player_id, PlayerArrowLevel::FullArrow);
				GenerateArrowOpcode(_player_id, _input->position_, Direction::South, PlayerArrowLevel::FullArrow);
			}
			break;
		case Opcodes::SendInput::ACT_EAST:
			if(arrow_present)
			{
				if(arrow.player_id == _player_id)
				{
					if(arrow.direction == Direction::East) //Clear
					{
						SetPlayerArrow(_input->position_, Direction::Stopped, -1, PlayerArrowLevel::Clear);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::Stopped, PlayerArrowLevel::Clear);
					}
					else //Replace
					{
						SetPlayerArrow(_input->position_, Direction::East, _player_id, PlayerArrowLevel::FullArrow);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::East, PlayerArrowLevel::FullArrow);
					}
				}
			} else if(CountArrows(_player_id) < arrow_limit_)
			{//New arrow
				SetPlayerArrow(_input->position_, Direction::East, _player_id, PlayerArrowLevel::FullArrow);
				GenerateArrowOpcode(_player_id, _input->position_, Direction::East, PlayerArrowLevel::FullArrow);
			}
			break;
		case Opcodes::SendInput::ACT_WEST:
			if(arrow_present)
			{
				if(arrow.player_id == _player_id)
				{
					if(arrow.direction == Direction::West) //Clear
					{
						SetPlayerArrow(_input->position_, Direction::Stopped, -1, PlayerArrowLevel::Clear);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::Stopped, PlayerArrowLevel::Clear);
					}
					else //Replace
					{
						SetPlayerArrow(_input->position_, Direction::West, _player_id, PlayerArrowLevel::FullArrow);
						GenerateArrowOpcode(_player_id, _input->position_, Direction::West, PlayerArrowLevel::FullArrow);
					}
				}
			} else if(CountArrows(_player_id) < arrow_limit_)
			{//New arrow
				SetPlayerArrow(_input->position_, Direction::West, _player_id, PlayerArrowLevel::FullArrow);
				GenerateArrowOpcode(_player_id, _input->position_, Direction::West, PlayerArrowLevel::FullArrow);
			}
			break;
		case Opcodes::SendInput::ACT_CLEAR:
			if(arrow_present)
			{
				if(arrow.player_id == _player_id)
				{
					SetPlayerArrow(_input->position_, Direction::Stopped, -1, PlayerArrowLevel::Clear);
					GenerateArrowOpcode(_player_id, _input->position_, Direction::Stopped, PlayerArrowLevel::Clear);
				}
			}
			break;
		}
	}
}


vector<Opcodes::ServerOpcode*> ServerWorld::GetOpcodes()
{
	vector<Opcodes::ServerOpcode*> opcodes_copy = opcodes_to_clients_;
	opcodes_to_clients_.clear();
	return opcodes_copy;
}


int ServerWorld::CountArrows(int _player_id)
{
	int count = 0;
	for(vector<PlayerArrow>::iterator it = player_arrows_.begin(); it != player_arrows_.end(); ++it)
	{
		if(it->player_id == _player_id) count++;
	}
	return count;
}

void ServerWorld::SpawnWalkers()
{
	float mouse_thresh = 200;
	float cat_thresh = mouse_thresh + 10;

	float total_thresh = cat_thresh;

	for(vector<Spawner>::iterator it = spawners_.begin(); it != spawners_.end(); ++it)
	{
		float value = Random::RandomRange(0, total_thresh);
		Walker* walker = new Walker();
		walker->SetPosition(it->position);
		walker->SetDirection(it->direction);
		walker->SetID(walker_id_cnt_);
		walker->SetWorld(this);
		if(value < mouse_thresh)
		{
			AddMouse(walker);
		} else //Else cat
		{
			AddCat(walker);
		}
		GenerateWalkerSpawn(walker);
		walker_id_cnt_++;
	}
}