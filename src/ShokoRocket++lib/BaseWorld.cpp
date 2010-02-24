#include "BaseWorld.h"
#include "Walker.h"
#include <TinyXML.h>
#include <algorithm>

namespace SquareType
{
	Direction::Enum GetDirection(Enum _square_type)
	{
		switch(_square_type)
		{
		case NorthArrow:
			return Direction::North;
		case SouthArrow:
			return Direction::South;
		case WestArrow:
			return Direction::West;
		case EastArrow:
			return Direction::East;
		case HalfNorthArrow:
			return Direction::North;
		case HalfSouthArrow:
			return Direction::South;
		case HalfWestArrow:
			return Direction::West;
		case HalfEastArrow:
			return Direction::East;
		default:
			return Direction::Stopped;
		}
	}

	Enum FromDirection(Direction::Enum _direction)
	{
		switch(_direction)
		{
		case Direction::North:
			return NorthArrow;
		case Direction::South:
			return SouthArrow;
		case Direction::West:
			return WestArrow;
		case Direction::East:
			return EastArrow;
		default:
			return Empty;
		}
	}

	Enum Diminish(Enum _square_type)
	{
		switch(_square_type)
		{
		case NorthArrow:
			return SquareType::HalfNorthArrow;
		case SouthArrow:
			return SquareType::HalfSouthArrow;
		case WestArrow:
			return SquareType::HalfWestArrow;
		case EastArrow:
			return SquareType::HalfEastArrow;
		case HalfNorthArrow:
			return SquareType::DestroyedNorthArrow;
		case HalfSouthArrow:
			return SquareType::DestroyedSouthArrow;
		case HalfWestArrow:
			return SquareType::DestroyedWestArrow;
		case HalfEastArrow:
			return SquareType::DestroyedEastArrow;
		default:
			return SquareType::Empty;
		}
	}
	Enum RestoreToFull(Enum _square_type)
	{
		switch(_square_type)
		{
		case HalfNorthArrow:
			return NorthArrow;
		case HalfSouthArrow:
			return SouthArrow;
		case HalfWestArrow:
			return WestArrow;
		case HalfEastArrow:
			return EastArrow;
		case DestroyedNorthArrow:
			return NorthArrow;
		case DestroyedSouthArrow:
			return SouthArrow;
		case DestroyedWestArrow:
			return WestArrow;
		case DestroyedEastArrow:
			return EastArrow;
		default:
			return _square_type;
		}
	}
}

BaseWorld::BaseWorld()
{
	size_ = Vector2i(20, 20);
	walls_ = vector<vector<TopLeft>>(size_.x, vector<TopLeft>(size_.y));
	special_squares_ = vector<vector<SquareType::Enum>>(size_.x, vector<SquareType::Enum>(size_.y));
	for(int x = 0; x < size_.x; x++)
		walls_[x][0].top = true;
	for(int y = 0; y < size_.y; y++)
		walls_[0][y].left = true;
	name_ = "Default 20x20";
	state_ = WorldState::OK;
}

BaseWorld::~BaseWorld()
{
	//Merge dead walkers into regular lists then free the lot
	just_dead_cats_.clear();
	just_dead_mice_.clear();

	mice_.insert(mice_.end(), dead_mice_.begin(), dead_mice_.end());
	dead_mice_.clear();

	cats_.insert(cats_.end(), dead_cats_.begin(), dead_cats_.end());
	dead_cats_.clear();

	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		 delete *it;
	}
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		 delete *it;
	}
}

void BaseWorld::LoadXML(TiXmlHandle& document_handle)
{
	TiXmlElement* pLevel = document_handle.FirstChild("Level").Element();
	TiXmlElement* pName = document_handle.FirstChild("Level").FirstChild("Name").Element();
	TiXmlElement* pSize = document_handle.FirstChild("Level").FirstChild("Size").Element();
	if(pLevel)
	{
		if(pName)
			name_ = pName->GetText();
		else
			name_ = "Unnamed level";

		if(pSize)
		{
			if(pSize->QueryIntAttribute("x", &size_.x) != TIXML_SUCCESS)
				state_ = WorldState::FileLoadError;
			if(pSize->QueryIntAttribute("y", &size_.y) != TIXML_SUCCESS)
				state_ = WorldState::FileLoadError;
			if(state_ == WorldState::OK)
			{
				walls_ = vector<vector<TopLeft>>(size_.x, vector<TopLeft>(size_.y));
				special_squares_ = vector<vector<SquareType::Enum>>(size_.x, vector<SquareType::Enum>(size_.y));
			}
		} else
			state_ = WorldState::FileLoadError;
		//Read list of 'H' elements representing horizontal walls
		TiXmlElement* h = document_handle.FirstChild("Level").FirstChild("H").Element();
		while(h)
		{
			Vector2i point;
			bool attribute_error = false;
			attribute_error |= (h->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
			attribute_error |= (h->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
			if(!attribute_error)
			{ 
				walls_[point.x][point.y].top = true;
			} else
			{}//TODO error 
			h = h->NextSiblingElement("H");
		}

		//Read list of 'V' elements representing vertical walls
		TiXmlElement* v = document_handle.FirstChild("Level").FirstChild("V").Element();
		while(v)
		{
			Vector2i point;
			bool attribute_error = false;
			attribute_error |= (v->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
			attribute_error |= (v->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
			if(!attribute_error)
			{ 
				walls_[point.x][point.y].left = true;
			} else
			{}//TODO error 
			v = v->NextSiblingElement("V");
		}

		//Load all holes
		TiXmlElement* hole = document_handle.FirstChild("Level").FirstChild("Hole").Element();
		while(hole)
		{
			Vector2i point;
			bool attribute_error = false;
			attribute_error |= (hole->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
			attribute_error |= (hole->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
			
			if(!attribute_error)
			{ 
				SetSquareType(point, SquareType::Hole);
				
			} else
			{}//TODO error 
			hole = hole->NextSiblingElement("Hole");
		}

	} else
	{
		state_ = WorldState::FileLoadError;
		//TODO error
	}
}

WorldState::Enum BaseWorld::Tick(float _dt)
{
	//Tick dead cats and mice
	for(vector<Walker*>::iterator it = dead_mice_.begin(); it != dead_mice_.end(); ++it)
	{
		(*it)->DeathTick(_dt);
	}
	for(vector<Walker*>::iterator it = dead_cats_.begin(); it != dead_cats_.end(); ++it)
	{
		(*it)->DeathTick(_dt);
	}
	return state_;
}

void BaseWorld::SetSize(Vector2i _size)
{
	size_ = _size;
	walls_ = vector<vector<TopLeft>>(size_.x, vector<TopLeft>(size_.y));
	for(int x = 0; x < size_.x; x++)
		walls_[x][0].top = true;
	for(int y = 0; y < size_.y; y++)
		walls_[0][y].left = true;
	special_squares_ = vector<vector<SquareType::Enum>>(size_.x, vector<SquareType::Enum>(size_.y));
}

GridSquare BaseWorld::GetGridSquare(Vector2i _point)
{
	if(!(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y))
		assert(false);
	GridSquare gs;
	gs.SetNorth(walls_[_point.x][_point.y].top);
	gs.SetWest(walls_[_point.x][_point.y].left);
	//Ugly ternery operators are wrap arounds 
	gs.SetSouth(walls_[_point.x][_point.y == size_.y - 1 ? 0 : _point.y + 1].top);
	gs.SetEast(walls_[_point.x == size_.x - 1 ? 0 : _point.x + 1][_point.y].left);

	return gs;
}



void BaseWorld::SetGridSquare(Vector2i _point, GridSquare _gs)
{
	assert(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y);
	walls_[_point.x][_point.y].top = _gs.GetNorth();
	walls_[_point.x][_point.y].left = _gs.GetWest();
	//Ugly ternery operators are wrap arounds 
	walls_[_point.x][_point.y == size_.y - 1 ? 0 : _point.y + 1].top = _gs.GetSouth();
	walls_[_point.x == size_.x - 1 ? 0 : _point.x + 1][_point.y].left = _gs.GetEast();
}

void BaseWorld::ToggleWall(Vector2i _position, Direction::Enum _direction)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	GridSquare gs = GetGridSquare(_position);
	gs.ToggleDirection(_direction);
	SetGridSquare(_position, gs);
}

SquareType::Enum BaseWorld::GetSquareType(Vector2i _point)
{
	assert(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y);
	return special_squares_[_point.x][_point.y];
}

bool BaseWorld::SetSquareType(Vector2i _point, SquareType::Enum _square_type)
{
	assert(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y);
	if(SquareType::GetDirection(_square_type) != Direction::Stopped)
	{
		if(special_squares_[_point.x][_point.y] == SquareType::Empty)
		{
			special_squares_[_point.x][_point.y] = _square_type;
			//TODO diminish stock
			return true;
		} else if(SquareType::GetDirection(special_squares_[_point.x][_point.y]) != Direction::Stopped)
		{
			//TODO free and then diminish stock
			special_squares_[_point.x][_point.y] = _square_type;
			return true;
		}
		return false;
	} else
	{
		special_squares_[_point.x][_point.y] = _square_type;
		return true;
	}
}

void BaseWorld::ToggleRocket(Vector2i _position)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	if(special_squares_[_position.x][_position.y] == SquareType::Rocket)
		special_squares_[_position.x][_position.y] = SquareType::Empty;
	else
		special_squares_[_position.x][_position.y] = SquareType::Rocket;
}

void BaseWorld::ToggleHole(Vector2i _position)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	if(special_squares_[_position.x][_position.y] == SquareType::Hole)
		special_squares_[_position.x][_position.y] = SquareType::Empty;
	else
		special_squares_[_position.x][_position.y] = SquareType::Hole;
}



void BaseWorld::AddMouse(Vector2i _position, Direction::Enum _direction)
{
	Walker* pMouse = new Walker();
	pMouse->SetWorld(this);
	pMouse->SetPosition(_position);
	pMouse->SetDirection(_direction);
	pMouse->SetWalkerType(WalkerType::Mouse);
	mice_.push_back(pMouse);
}

void BaseWorld::AddMouse(Walker* _pMouse)
{
	_pMouse->SetWorld(this);
	_pMouse->SetWalkerType(WalkerType::Mouse);
	mice_.push_back(_pMouse);
}


void BaseWorld::ToggleMouse(Vector2i _position, Direction::Enum _direction)
{
	Vector2f fpos = _position;
	Walker* mouse_to_remove = NULL;
	Walker* cat_to_remove = NULL;
	Walker* mouse_to_add = NULL;
	for(std::vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); it++)
	{
		if(((*it)->GetPosition() - fpos).length() < 0.01f)
		{
			mouse_to_remove = *it;
			if((*it)->GetDirection() != _direction)
			{
				mouse_to_add = new Walker();
				mouse_to_add->SetWorld(this);
				mouse_to_add->SetWalkerType(WalkerType::Mouse);
				mouse_to_add->SetPosition(fpos);
				mouse_to_add->SetDirection(_direction);
			}
		}
	}
	if(!mouse_to_remove)
	{
		for(std::vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); it++)
		{
			if(((*it)->GetPosition() - fpos).length() < 0.01f)
			{
				cat_to_remove = *it;
			}
		}
		mouse_to_add = new Walker();
		mouse_to_add->SetWorld(this);
		mouse_to_add->SetWalkerType(WalkerType::Mouse);
		mouse_to_add->SetPosition(fpos);
		mouse_to_add->SetDirection(_direction);
	}
	if(mouse_to_remove)
		mice_.erase(std::remove(mice_.begin(), mice_.end(), mouse_to_remove));
	if(mouse_to_add)
		mice_.push_back(mouse_to_add);
	if(cat_to_remove)
		cats_.erase(std::remove(cats_.begin(), cats_.end(), cat_to_remove));
}

void BaseWorld::AddCat(Vector2i _position, Direction::Enum _direction)
{
	Walker* pCat = new Walker();
	pCat->SetWorld(this);
	pCat->SetPosition(_position);
	pCat->SetDirection(_direction);
	pCat->SetWalkerType(WalkerType::Cat);
	cats_.push_back(pCat);
}

void BaseWorld::AddCat(Walker* _pCat)
{
	_pCat->SetWorld(this);	
	_pCat->SetWalkerType(WalkerType::Cat);
	cats_.push_back(_pCat);
}

void BaseWorld::ToggleCat(Vector2i _position, Direction::Enum _direction)
{
	Vector2f fpos = _position;
	Walker* cat_to_remove = NULL;
	Walker* mouse_to_remove = NULL;
	Walker* cat_to_add = NULL;
	for(std::vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); it++)
	{
		if(((*it)->GetPosition() - fpos).length() < 0.01f)
		{
			cat_to_remove = *it;
			if((*it)->GetDirection() != _direction)
			{
				cat_to_add = new Walker();
				cat_to_add->SetWorld(this);
				cat_to_add->SetWalkerType(WalkerType::Cat);
				cat_to_add->SetPosition(fpos);
				cat_to_add->SetDirection(_direction);
			}
		}
	}
	if(!cat_to_remove)
	{
		for(std::vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); it++)
		{
			if(((*it)->GetPosition() - fpos).length() < 0.01f)
			{
				mouse_to_remove = *it;
			}
		}
		cat_to_add = new Walker();
		cat_to_add->SetWorld(this);
		cat_to_add->SetWalkerType(WalkerType::Cat);
		cat_to_add->SetPosition(fpos);
		cat_to_add->SetDirection(_direction);
	}
	if(cat_to_remove)
		cats_.erase(std::remove(cats_.begin(), cats_.end(), cat_to_remove));
	if(cat_to_add)
		cats_.push_back(cat_to_add);
	if(mouse_to_remove)
		mice_.erase(std::remove(mice_.begin(), mice_.end(), mouse_to_remove));
}



float BaseWorld::GetShortestDistance(Vector2f _position1, Vector2f _position2, Vector2f _wrap_size)
{
	/* Rather than do comparison 0-10 etc map each coordinate to -5 to +5 
	eg: 1 -> 1
	    5 -> 5
		6 -> -4
		7 -> -3
		9 -> -1
	*/
	float d1 = (_position1 - _position2).length();
	Vector2f wrap1 = _position1;
	if(wrap1.x > _wrap_size.x/2)
		wrap1.x -= _wrap_size.x;
	if(wrap1.y > _wrap_size.y/2)
		wrap1.y -= _wrap_size.y;
	Vector2f wrap2 = _position2;
	if(wrap2.x > _wrap_size.x/2)
		wrap2.x -= _wrap_size.x;
	if(wrap2.y > _wrap_size.y/2)
		wrap2.y -= _wrap_size.y;
	float d2 = (wrap1 - wrap2).length();
	if(d1 == 0 || d2 == 0)
		return 0;
	if(d1 < d2)
		return d1;
	else
		return d2;
}
