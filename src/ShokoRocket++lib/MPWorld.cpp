#include "MPWorld.h"
#include <TinyXML.h>
#include <algorithm>
#include "Walker.h"
#include "Logger.h"


MPWorld::MPWorld() : BaseWorld()
{
	time_ = 0;
}

MPWorld::MPWorld(string _filename) : BaseWorld()
{
	time_ = 0;
	filename_ = _filename;
	_filename = "Levels/" + _filename; 
	state_ = WorldState::OK;
	TiXmlDocument document = TiXmlDocument(_filename.c_str());	
	if(document.LoadFile())
	{
		TiXmlHandle document_handle = TiXmlHandle(&document);
		LoadXML(document_handle);
	} else
	{
		state_ = WorldState::FileLoadError;
		size_ = Vector2i(20, 20);
		walls_ = vector<vector<TopLeft>>(size_.x, vector<TopLeft>(size_.y));
		special_squares_ = vector<vector<SquareType::Enum>>(size_.x, vector<SquareType::Enum>(size_.y));
		name_ = "Unnamed level - load failed";
		//TODO Log error not worked, load default
	}

}

void MPWorld::LoadXML(TiXmlHandle& document_handle)
{
	BaseWorld::LoadXML(document_handle);
	//Read list of 'PlayerRocket' elements
	TiXmlElement* playerrocket = document_handle.FirstChild("Level").FirstChild("PlayerRocket").Element();
	while(playerrocket)
	{
		Vector2i point;
		int player_id;
		bool attribute_error = false;
		attribute_error |= (playerrocket->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
		attribute_error |= (playerrocket->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
		attribute_error |= (playerrocket->QueryIntAttribute("player_id", &player_id) != TIXML_SUCCESS);
		if(!attribute_error)
		{ 			
			SetPlayerRocket(point, player_id);
			
		} else
		{}//TODO error 

		playerrocket = playerrocket->NextSiblingElement("PlayerRocket");
	}
	//Read list of 'Spawner' elements
	TiXmlElement* spawner = document_handle.FirstChild("Level").FirstChild("Spawner").Element();
	while(spawner)
	{
		Vector2i point;
		string direction_string = "South";
		bool attribute_error = false;
		attribute_error |= (spawner->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
		attribute_error |= (spawner->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
		attribute_error |= (spawner->QueryValueAttribute("d", &direction_string) != TIXML_SUCCESS);
		
		if(!attribute_error)
		{ 
			Direction::Enum direction = Direction::FromString(direction_string);
			SetSpawner(point, direction);
			
		} else
		{}//TODO error 

		spawner = spawner->NextSiblingElement("Spawner");
	}
}
WorldState::Enum MPWorld::Tick(float _dt)
{
	unsigned int dt_ms = static_cast<unsigned int>(_dt * 1000);
	time_ += dt_ms;


	const float max_dt = 0.1f;
	int full_iterations = floor(_dt/max_dt);
	float last_iteration = _dt - full_iterations * max_dt;
	for(int i = 0; i < full_iterations + 1; i++)
	{
		float dt;
		if(i == full_iterations)
			dt = last_iteration;
		else
			dt = max_dt;
		if(dt <= 0)
			continue;
		if(state_ == WorldState::OK)
		{
			//Remove cats and mice that have been instructed to die
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

			//Walk forward
			for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
			{
				(*it)->Advance(dt);
			}
			for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
			{
				(*it)->Advance(dt);
			}
		}
	}

	BaseWorld::Tick(_dt);
	return state_;
}
void MPWorld::WalkerReachNewSquare(Walker* _walker)
{
}

void MPWorld::SetSpawner(Vector2i _position, Direction::Enum _direction)
{
	//Either remove, update or create a spawner
	bool found_or_removed = false;
	for(vector<Spawner>::iterator it = spawners_.begin(); it != spawners_.end(); ++it)
	{
		if(it->position == _position)
		{
			if(_direction == Direction::Stopped)
			{
				spawners_.erase(it);
				found_or_removed = true;
				break;				
			} else
			{
				it->direction = _direction;
				found_or_removed = true;
			}
		}
	}
	if(!found_or_removed)
	{
		Spawner sp;
		sp.position = _position;
		sp.direction = _direction;
		spawners_.push_back(sp);
	}

}
void MPWorld::ToggleSpawner(Vector2i _position, Direction::Enum _direction)
{
	bool toggled_or_removed = false;
	for(vector<Spawner>::iterator it = spawners_.begin(); it != spawners_.end(); it++)
	{
		if(it->position == _position)
		{
			if(it->direction != _direction)
			{
				it->direction = _direction;
				toggled_or_removed = true;
			} else
			{
				spawners_.erase(it);
				toggled_or_removed = true;
				break;
			}
		}
	}
	if(!toggled_or_removed)
	{
		Spawner sp;
		sp.position = _position;
		sp.direction = _direction;
		spawners_.push_back(sp);
	}
}

void MPWorld::SetPlayerRocket(Vector2i _position, int _player_id)
{
	bool updated = false;
	for(vector<PlayerRocket>::iterator it = player_rockets_.begin(); it != player_rockets_.end(); ++it)
	{
		if(it->player_id == _player_id)
		{
			it->position = _position;
			updated = true;
		}
	}
	if(!updated)
	{
		PlayerRocket pr;
		pr.player_id = _player_id;
		pr.position = _position;
		player_rockets_.push_back(pr);
	}
}

void MPWorld::SetPlayerArrow(Vector2i _position, Direction::Enum _direction, int _player_id, PlayerArrowLevel::Enum _arrow_level)
{
	bool updated = false;
	for(vector<PlayerArrow>::iterator it = player_arrows_.begin(); it != player_arrows_.end(); ++it)
	{
		if(it->position == _position)
		{
			if(_arrow_level == PlayerArrowLevel::Clear)
			{
				updated = true;
				player_arrows_.erase(it);
				break;
			}
			if(_arrow_level == PlayerArrowLevel::HalfArrow) it->halved = true;
			if(_arrow_level == PlayerArrowLevel::FullArrow) it->halved = false;
			it->direction = _direction;
			it->player_id = _player_id;
			updated = true;
		}
	}
	if(!updated)
	{
		PlayerArrow pa;
		pa.direction = _direction;
		pa.player_id = _player_id;
		pa.position = _position;
		if(_arrow_level == PlayerArrowLevel::FullArrow) pa.halved = false;
		if(_arrow_level == PlayerArrowLevel::HalfArrow) pa.halved = true;
		if(_arrow_level == PlayerArrowLevel::Clear) return;
		player_arrows_.push_back(pa);
	}
}




void MPWorld::UpdateCat(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time)
{
	//Find cat by ID
	Walker* cat = NULL;
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		if((*it)->GetID() == _id)
		{
			cat = *it;
		}
	}
	if(!cat)
	{
		Logger::DiagnosticOut() << "Probable sync issue, tried to update Cat " << _id << " but could not find\n";
		return;
	}

	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		cat->SetPosition(_position);
		cat->SetDirection(_direction);
	} else
	{
		//Set position/direction to updated data
		cat->SetPosition(_position);
		cat->SetDirection(_direction);

		float dt = static_cast<float>(time_ - _time) / 1000.0f;
		cat->Advance(dt);
	}	
}
void MPWorld::UpdateMouse(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time)
{
	//Find mouse by ID
	Walker* mouse = NULL;
	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		if((*it)->GetID() == _id)
		{
			mouse = *it;
		}
	}
	if(!mouse)
	{
		Logger::DiagnosticOut() << "Probable sync issue, tried to update Mouse " << _id << " but could not find\n";
		return;
	}

	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		mouse->SetDirection(_direction);
	} else
	{
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		mouse->SetDirection(_direction);

		float dt = static_cast<float>(time_ - _time) / 1000.0f;
		mouse->Advance(dt);
	}
}

void MPWorld::KillMouse(unsigned int _id, Vector2f _position, unsigned int _time)
{
	//Find mouse by ID
	Walker* mouse = NULL;
	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		if((*it)->GetID() == _id)
		{
			mouse = *it;
		}
	}
	if(!mouse)
	{
		Logger::DiagnosticOut() << "Probable sync issue, tried to kill Mouse " << _id << " but could not find\n";
		return;
	}
	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		just_dead_mice_.push_back(mouse);
	} else
	{
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		just_dead_mice_.push_back(mouse);
	}
}

void MPWorld::KillCat(unsigned int _id, Vector2f _position, unsigned int _time)
{
	//Find cat by ID
	Walker* cat = NULL;
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		if((*it)->GetID() == _id)
		{
			cat= *it;
		}
	}
	if(!cat)
	{
		Logger::DiagnosticOut() << "Probable sync issue, tried to kill Cat " << _id << " but could not find\n";
		return;
	}
	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		cat->SetPosition(_position);
		just_dead_cats_.push_back(cat);
	} else
	{
		//Set position/direction to updated data
		cat->SetPosition(_position);
		just_dead_cats_.push_back(cat);
	}

}

void MPWorld::CreateMouse(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time)
{
	Walker* mouse = new Walker();
	mouse->SetID(_id);
	mouse->SetWorld(this);
	mouse->SetWalkerType(WalkerType::Mouse);
	mice_.push_back(mouse);

	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		mouse->SetDirection(_direction);
	} else
	{
		//Set position/direction to updated data
		mouse->SetPosition(_position);
		mouse->SetDirection(_direction);
		float dt = static_cast<float>(time_ - _time) / 1000.0f;
		mouse->Advance(dt);
	}
}

void MPWorld::CreateCat(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time)
{
	Walker* cat = new Walker();
	cat->SetID(_id);
	cats_.push_back(cat);
	cat->SetWalkerType(WalkerType::Cat);
	cat->SetWorld(this);

	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
		//Set position/direction to updated data
		cat->SetPosition(_position);
		cat->SetDirection(_direction);
	} else
	{
		//Set position/direction to updated data
		cat->SetPosition(_position);
		cat->SetDirection(_direction);
		float dt = static_cast<float>(time_ - _time) / 1000.0f;
		cat->Advance(dt);
	}
}

void MPWorld::RemoveWalker(unsigned int _id, bool _kill, Vector2f _position, unsigned int _time)
{
	Walker* walker = NULL;
	WalkerType::Enum walker_type;
	//Find mouse by ID
	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		if((*it)->GetID() == _id)
		{
			walker = *it;
			walker_type = WalkerType::Mouse;
			break;
		}
	}
	if(!walker)
	{
		for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
		{
			if((*it)->GetID() == _id)
			{
				walker = *it;
				walker_type = WalkerType::Cat;
				break;
			}
		}
	}
	if(!walker)
	{
		Logger::DiagnosticOut() << "Probable sync issue, tried to remove walker " << _id << " but could not find\n";
		return;
	}
	//Advance to current time
	if(time_ < _time)
	{
		Logger::DiagnosticOut() << "Received an update from the future, client must be running slow. Flagging for fast forward\n";
		Logger::DiagnosticOut() << "Local time: " << time_ << " Remote time: " << _time << "\n";
		//Fast forward to resync to server
		float dt = static_cast<float>(_time - time_) / 1000.0f;
		Tick(dt);
	}
	//Set position to updated data
	walker->SetPosition(_position);
	switch(walker_type)
	{
	case WalkerType::Mouse:
		just_dead_mice_.push_back(walker);
		if(_kill) 
			walker->Kill();
		else
			walker->Rescue();
		break;
	case WalkerType::Cat:
		just_dead_cats_.push_back(walker);
		walker->Kill();
		break;
	}

}