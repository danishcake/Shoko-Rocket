#include "World.h"
#include <assert.h>
#include <TinyXML.h>
#include <algorithm>
#include "Walker.h"

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

World::World(void)
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
	rescued_mice_ = 0;
	unlimited_arrow_stock_ = false;
}

World::World(std::string _filename)
{
	filename_ = _filename;
	_filename = "Levels/" + _filename; 
	state_ = WorldState::OK;
	rescued_mice_ = 0;
	unlimited_arrow_stock_ = false;
	TiXmlDocument document = TiXmlDocument(_filename.c_str());
	TiXmlHandle document_handle = TiXmlHandle(&document);
	if(document.LoadFile())
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
			//Read list of 'Mouse' elements
			TiXmlElement* mouse = document_handle.FirstChild("Level").FirstChild("Mouse").Element();
			while(mouse)
			{
				Vector2i point;
				string direction_string = "South";
				bool attribute_error = false;
				attribute_error |= (mouse->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
				attribute_error |= (mouse->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
				attribute_error |= (mouse->QueryValueAttribute("d", &direction_string) != TIXML_SUCCESS);
				
				if(!attribute_error)
				{ 
					Direction::Enum direction = Direction::FromString(direction_string);
					AddMouse(point, direction);
					
				} else
				{}//TODO error 
				mouse = mouse->NextSiblingElement("Mouse");
			}
			//Read list of 'Cat' elements
			TiXmlElement* cat = document_handle.FirstChild("Level").FirstChild("Cat").Element();
			while(cat)
			{
				Vector2i point;
				string direction_string = "South";
				bool attribute_error = false;
				attribute_error |= (cat->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
				attribute_error |= (cat->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
				attribute_error |= (cat->QueryValueAttribute("d", &direction_string) != TIXML_SUCCESS);
				
				if(!attribute_error)
				{ 
					Direction::Enum direction = Direction::FromString(direction_string);
					AddCat(point, direction);
					
				} else
				{}//TODO error 
				cat = cat->NextSiblingElement("Cat");
			}

			//Load all rockets
			TiXmlElement* rocket = document_handle.FirstChild("Level").FirstChild("Rocket").Element();
			while(rocket)
			{
				Vector2i point;
				bool attribute_error = false;
				attribute_error |= (rocket->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
				attribute_error |= (rocket->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
				
				if(!attribute_error)
				{ 
					SetSquareType(point, SquareType::Rocket);
					
				} else
				{}//TODO error 
				rocket = rocket->NextSiblingElement("Rocket");
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
			//Read list of 'Arrow' elements
			TiXmlElement* arrow = document_handle.FirstChild("Level").FirstChild("Arrow").Element();
			while(arrow)
			{
				Vector2i point;
				string direction_string = "South";
				bool attribute_error = false;
				attribute_error |= (arrow->QueryIntAttribute("x", &point.x) != TIXML_SUCCESS);
				attribute_error |= (arrow->QueryIntAttribute("y", &point.y) != TIXML_SUCCESS);
				attribute_error |= (arrow->QueryValueAttribute("d", &direction_string) != TIXML_SUCCESS);
				
				if(!attribute_error)
				{ 
					Direction::Enum direction = Direction::FromString(direction_string);
					AddArrow(direction); //Todo save solution
					ArrowRecord ar;
					ar.Direction = direction;
					ar.Position = point;
					solution_arrows_.push_back(ar);
					
				} else
				{}//TODO error 
				arrow = arrow->NextSiblingElement("Arrow");
			}
		} else
		{
			state_ = WorldState::FileLoadError;
			//TODO error
		}
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

void World::SetSize(Vector2i _size)
{
	size_ = _size;
	walls_ = vector<vector<TopLeft>>(size_.x, vector<TopLeft>(size_.y));
	for(int x = 0; x < size_.x; x++)
		walls_[x][0].top = true;
	for(int y = 0; y < size_.y; y++)
		walls_[0][y].left = true;
	special_squares_ = vector<vector<SquareType::Enum>>(size_.x, vector<SquareType::Enum>(size_.y));
}

World::~World(void)
{
	Reset(); //Moves dead mice and cats to mice_ and cats_
	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		 delete *it;
	}
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		 delete *it;
	}
}

GridSquare World::GetGridSquare(Vector2i _point)
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

void World::SetGridSquare(Vector2i _point, GridSquare _gs)
{
	assert(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y);
	walls_[_point.x][_point.y].top = _gs.GetNorth();
	walls_[_point.x][_point.y].left = _gs.GetWest();
	//Ugly ternery operators are wrap arounds 
	walls_[_point.x][_point.y == size_.y - 1 ? 0 : _point.y + 1].top = _gs.GetSouth();
	walls_[_point.x == size_.x - 1 ? 0 : _point.x + 1][_point.y].left = _gs.GetEast();
}

void World::ToggleWall(Vector2i _position, Direction::Enum _direction)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	GridSquare gs = GetGridSquare(_position);
	gs.ToggleDirection(_direction);
	SetGridSquare(_position, gs);
}

SquareType::Enum World::GetSquareType(Vector2i _point)
{
	assert(_point.x >= 0 && _point.x < size_.x && _point.y >= 0	&& _point.y < size_.y);
	return special_squares_[_point.x][_point.y];
}

bool World::SetSquareType(Vector2i _point, SquareType::Enum _square_type)
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

void World::ToggleRocket(Vector2i _position)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	if(special_squares_[_position.x][_position.y] == SquareType::Rocket)
		special_squares_[_position.x][_position.y] = SquareType::Empty;
	else
		special_squares_[_position.x][_position.y] = SquareType::Rocket;
}

void World::ToggleHole(Vector2i _position)
{
	assert(_position.x >= 0 && _position.x < size_.x && _position.y >= 0	&& _position.y < size_.y);
	if(special_squares_[_position.x][_position.y] == SquareType::Hole)
		special_squares_[_position.x][_position.y] = SquareType::Empty;
	else
		special_squares_[_position.x][_position.y] = SquareType::Hole;
}


void World::AddMouse(Vector2i _position, Direction::Enum _direction)
{
	Walker* pMouse = new Walker();
	pMouse->SetWorld(this);
	pMouse->SetPosition(_position);
	pMouse->SetDirection(_direction);
	pMouse->SetWalkerType(WalkerType::Mouse);
	mice_.push_back(pMouse);
}

void World::AddMouse(Walker* _pMouse)
{
	_pMouse->SetWorld(this);
	_pMouse->SetWalkerType(WalkerType::Mouse);
	mice_.push_back(_pMouse);
}

void World::ToggleMouse(Vector2i _position, Direction::Enum _direction)
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

void World::ToggleCat(Vector2i _position, Direction::Enum _direction)
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

void World::AddCat(Vector2i _position, Direction::Enum _direction)
{
	Walker* pCat = new Walker();
	pCat->SetWorld(this);
	pCat->SetPosition(_position);
	pCat->SetDirection(_direction);
	pCat->SetWalkerType(WalkerType::Cat);
	cats_.push_back(pCat);
}

void World::AddCat(Walker* _pCat)
{
	_pCat->SetWorld(this);	
	_pCat->SetWalkerType(WalkerType::Cat);
	cats_.push_back(_pCat);
}

WorldState::Enum World::Tick(float _dt)
{
	//TODO perhaps split large DT into smaller periods?

	if(state_ == WorldState::OK)
	{
		//Walk forward
		for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
		{
			(*it)->Advance(_dt);
		}
		for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
		{
			(*it)->Advance(_dt);
		}
		//Collide cats and mice
		for(vector<Walker*>::iterator m_it = mice_.begin(); m_it != mice_.end(); ++m_it)
		{
			for(vector<Walker*>::iterator c_it = cats_.begin(); c_it != cats_.end(); ++c_it)
			{
				if(((*m_it)->GetPosition() - (*c_it)->GetPosition()).length() < 0.3333f)
				{
					if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), *m_it) == just_dead_mice_.end())
					{
						just_dead_mice_.push_back(*m_it);
						(*m_it)->Kill();
						state_ = WorldState::Defeat;
						problem_points_.push_back((*m_it)->GetPosition());
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
		if(state_ == WorldState::OK && rescued_mice_ > 0)
		{
			if(rescued_mice_ == GetTotalMice())
				state_ = WorldState::Victory;
		}
	}

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

void World::SaveAs(string _filename)
{
	Reset();
	_filename = "Levels/" + _filename; 
	TiXmlDocument document = TiXmlDocument(_filename);
	TiXmlHandle document_handle = TiXmlHandle(&document);
	TiXmlNode* pDeclaration = new TiXmlDeclaration();
	document.LinkEndChild(pDeclaration);
	TiXmlElement* pLevel = new TiXmlElement("Level");
	document.LinkEndChild(pLevel);
	TiXmlElement* pName = new TiXmlElement("Name");
	pLevel->LinkEndChild(pName);
	pName->LinkEndChild(new TiXmlText(name_));
	TiXmlElement* pSize = new TiXmlElement("Size");
	pSize->SetAttribute("x", size_.x);
	pSize->SetAttribute("y", size_.y);
	pLevel->LinkEndChild(pSize);
	

	for(int x = 0; x < size_.x; x++)
	{
		for(int y = 0; y < size_.y; y++)
		{
			if(walls_[x][y].left)
			{
				TiXmlElement* pLeftWall = new TiXmlElement("V");
				pLeftWall->SetAttribute("x", x);
				pLeftWall->SetAttribute("y", y);
				pLevel->LinkEndChild(pLeftWall);
			}
		}
	}
	for(int x = 0; x < size_.x; x++)
	{
		for(int y = 0; y < size_.y; y++)
		{
			if(walls_[x][y].top)
			{
				TiXmlElement* pTopWall = new TiXmlElement("H");
				pTopWall->SetAttribute("x", x);
				pTopWall->SetAttribute("y", y);
				pLevel->LinkEndChild(pTopWall);
			}
		}
	}
	for(int x = 0; x < size_.x; x++)
	{
		for(int y = 0; y < size_.y; y++)
		{
			if(special_squares_[x][y] == SquareType::Hole)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Hole");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pLevel->LinkEndChild(pSpecialSquare);
			} else if(special_squares_[x][y] == SquareType::Rocket)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Rocket");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pLevel->LinkEndChild(pSpecialSquare);
			} else if (special_squares_[x][y] == SquareType::WestArrow)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Arrow");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pSpecialSquare->SetAttribute("d", "West");
				pLevel->LinkEndChild(pSpecialSquare);
			} else if (special_squares_[x][y] == SquareType::EastArrow)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Arrow");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pSpecialSquare->SetAttribute("d", "East");
				pLevel->LinkEndChild(pSpecialSquare);
			} else if (special_squares_[x][y] == SquareType::NorthArrow)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Arrow");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pSpecialSquare->SetAttribute("d", "North");
				pLevel->LinkEndChild(pSpecialSquare);
			} else if (special_squares_[x][y] == SquareType::SouthArrow)
			{
				TiXmlElement* pSpecialSquare = new TiXmlElement("Arrow");
				pSpecialSquare->SetAttribute("x", x);
				pSpecialSquare->SetAttribute("y", y);
				pSpecialSquare->SetAttribute("d", "South");
				pLevel->LinkEndChild(pSpecialSquare);
			}

		}
	}

	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		TiXmlElement* pMouse = new TiXmlElement("Mouse");
		pMouse->SetAttribute("x", static_cast<int>((*it)->GetPosition().x));
		pMouse->SetAttribute("y", static_cast<int>((*it)->GetPosition().y));
		pMouse->SetAttribute("d", Direction::ToString((*it)->GetDirection()));
		pLevel->LinkEndChild(pMouse);
		
	}

	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		TiXmlElement* pCat = new TiXmlElement("Cat");
		pCat->SetAttribute("x", static_cast<int>((*it)->GetPosition().x));
		pCat->SetAttribute("y", static_cast<int>((*it)->GetPosition().y));
		pCat->SetAttribute("d", Direction::ToString((*it)->GetDirection()));
		pLevel->LinkEndChild(pCat);
		
	}
	document.SaveFile();
}

void World::ClearArrows()
{
	for(int x = 0; x < size_.x; x++)
	{
		for(int y= 0; y < size_.y; y++)
		{
			if(SquareType::GetDirection(special_squares_[x][y]) != Direction::Stopped)
			{
				if(!unlimited_arrow_stock_)
					arrows_.push_back(SquareType::GetDirection(special_squares_[x][y]));
				special_squares_[x][y] = SquareType::Empty;
			}
		}
	}
}

void World::LoadSolution()
{
	ClearArrows();
	for(vector<ArrowRecord>::iterator it = solution_arrows_.begin(); it != solution_arrows_.end(); ++it)
	{
		ToggleArrow(it->Position, it->Direction);
	}
}

void World::Reset()
{
	just_dead_cats_.clear();
	just_dead_mice_.clear();

	mice_.insert(mice_.end(), dead_mice_.begin(), dead_mice_.end());
	dead_mice_.clear();
	rescued_mice_ = 0;

	cats_.insert(cats_.end(), dead_cats_.begin(), dead_cats_.end());
	dead_cats_.clear();

	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		(*it)->Reset();
	}
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		(*it)->Reset();
	}

	for(int x = 0; x < size_.x; x++)
	{
		for(int y= 0; y < size_.y; y++)
		{
			special_squares_[x][y] = SquareType::RestoreToFull(special_squares_[x][y]);
		}
	}
	
	problem_points_.clear();

	if(state_ !=  WorldState::FileLoadError)
		state_ = WorldState::OK;
}

void World::WalkerReachNewSquare(Walker* _walker)
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
				problem_points_.push_back(_walker->GetPosition());
				_walker->Kill();
				state_ = WorldState::Defeat;
			}
		} else if(square_type == SquareType::Rocket)
		{
			if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), _walker) == just_dead_mice_.end())
			{
				just_dead_mice_.push_back(_walker);
				rescued_mice_++;
				_walker->Rescue();
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
			}
		}
		if(square_type == SquareType::Rocket)
		{
			if(std::find(just_dead_cats_.begin(), just_dead_cats_.end(), _walker) == just_dead_cats_.end())
			{
				just_dead_cats_.push_back(_walker);
				_walker->Kill();
			}
			state_ = WorldState::Defeat;
			problem_points_.push_back(_walker->GetPosition());
		}
		break;
	}
	
	
	if(SquareType::GetDirection(square_type) != Direction::Stopped)
	{
		Direction::Enum direction = SquareType::GetDirection(square_type);
		//Cat should destroy the arrow
		if(_walker->GetWalkerType() == WalkerType::Cat && SquareType::GetDirection(square_type) == Direction::TurnAround(_walker->GetDirection()))
		{
			special_squares_[position_grid.x][position_grid.y] = SquareType::Diminish(square_type);
			ArrowRecord ar;
			ar.Direction = SquareType::GetDirection(square_type);
			ar.Position = position_grid;
		}
		_walker->EncounterArrow(direction);
	}
}

void World::ToggleArrow(Vector2i _position, Direction::Enum _direction)
{
	assert(_direction != Direction::Stopped);
	//TODO bounds checking assertions
	if(SquareType::GetDirection(special_squares_[_position.x][_position.y]) == _direction)
	{
		if(!unlimited_arrow_stock_)
			arrows_.push_back(_direction);
		special_squares_[_position.x][_position.y] = SquareType::Empty;
		return;
	} else if(special_squares_[_position.x][_position.y] == SquareType::Empty)
	{
		if(!unlimited_arrow_stock_)
		{
			vector<Direction::Enum>::iterator arrow = std::find(arrows_.begin(), arrows_.end(), _direction);
			if(arrow != arrows_.end())
			{ //Have an arrow of this type
				special_squares_[_position.x][_position.y] = SquareType::FromDirection(_direction);
				arrows_.erase(arrow);
			}
		} else
		{
			special_squares_[_position.x][_position.y] = SquareType::FromDirection(_direction);
		}
	} else if(SquareType::GetDirection(special_squares_[_position.x][_position.y]) != Direction::Stopped)
	{
		if(!unlimited_arrow_stock_)
		{
			vector<Direction::Enum>::iterator arrow = std::find(arrows_.begin(), arrows_.end(), _direction);
			if(arrow != arrows_.end())
			{ //Have an arrow of this type
				arrows_.erase(arrow);	
				arrows_.push_back(SquareType::GetDirection(special_squares_[_position.x][_position.y]));
				special_squares_[_position.x][_position.y] = SquareType::FromDirection(_direction);
			}
		} else
		{
			special_squares_[_position.x][_position.y] = SquareType::FromDirection(_direction);
		}
	}
}

void World::ClearArrow(Vector2i _position)
{
	Direction::Enum direction = SquareType::GetDirection(special_squares_[_position.x][_position.y]);
	if(direction != Direction::Stopped)
	{
		if(!unlimited_arrow_stock_)
			arrows_.push_back(direction);
		special_squares_[_position.x][_position.y] = SquareType::Empty;

	}
}

