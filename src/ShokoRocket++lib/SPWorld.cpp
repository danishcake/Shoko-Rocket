#include "SPWorld.h"
#include <assert.h>
#include <TinyXML.h>
#include <algorithm>
#include "Walker.h"


SPWorld::SPWorld(void) : BaseWorld()
{
	rescued_mice_ = 0;
	unlimited_arrow_stock_ = false;
}

SPWorld::SPWorld(std::string _filename)
{
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
	/* I would have liked to have moved the above into BaseWorld but cannot call virtual fns 
	   from base class constructors */

	rescued_mice_ = 0;
	unlimited_arrow_stock_ = false;
}




void SPWorld::LoadXML(TiXmlHandle& document_handle)
{
	//Read common stuff - walls, size, name etc
	BaseWorld::LoadXML(document_handle);
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
}

WorldState::Enum SPWorld::Tick(float _dt)
{
	//TODO perhaps split large DT into smaller periods?
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
					if(SPWorld::GetShortestDistance((*m_it)->GetPosition(), (*c_it)->GetPosition(), size_) < 0.3333f)
					//if(((*m_it)->GetPosition() - (*c_it)->GetPosition()).length() < 0.3333f)
					{
						if(std::find(just_dead_mice_.begin(), just_dead_mice_.end(), *m_it) == just_dead_mice_.end())
						{
							just_dead_mice_.push_back(*m_it);
							(*m_it)->Kill();
							(*c_it)->SetProblem(true);
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
				(*it)->SetProblem(true);
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
	}
	BaseWorld::Tick(_dt);

	return state_;
}

void SPWorld::SaveAs(string _filename)
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

void SPWorld::ClearArrows()
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

void SPWorld::LoadSolution()
{
	ClearArrows();
	for(vector<ArrowRecord>::iterator it = solution_arrows_.begin(); it != solution_arrows_.end(); ++it)
	{
		ToggleArrow(it->Position, it->Direction);
	}
}

void SPWorld::Reset()
{
	rescued_mice_ = 0;

	just_dead_cats_.clear();
	just_dead_mice_.clear();

	mice_.insert(mice_.end(), dead_mice_.begin(), dead_mice_.end());
	dead_mice_.clear();

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

void SPWorld::WalkerReachNewSquare(Walker* _walker)
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

void SPWorld::ToggleArrow(Vector2i _position, Direction::Enum _direction)
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

void SPWorld::ClearArrow(Vector2i _position)
{
	Direction::Enum direction = SquareType::GetDirection(special_squares_[_position.x][_position.y]);
	if(direction != Direction::Stopped)
	{
		if(!unlimited_arrow_stock_)
			arrows_.push_back(direction);
		special_squares_[_position.x][_position.y] = SquareType::Empty;

	}
}

void SPWorld::RescueAllMice()
{
	for(std::vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		(*it)->Rescue();
		just_dead_mice_.push_back(*it);	
	}
	rescued_mice_ = mice_.size();
}

void SPWorld::KillAllMice()
{
	for(std::vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		(*it)->Kill();
		just_dead_mice_.push_back(*it);
	}
}

void SPWorld::ResetProblemWalkers()
{
	for(vector<Walker*>::iterator it = mice_.begin(); it != mice_.end(); ++it)
	{
		(*it)->SetProblem(false);
	}
	for(vector<Walker*>::iterator it = dead_mice_.begin(); it != dead_mice_.end(); ++it)
	{
		(*it)->SetProblem(false);
	}
	for(vector<Walker*>::iterator it = cats_.begin(); it != cats_.end(); ++it)
	{
		(*it)->SetProblem(false);
	}
	for(vector<Walker*>::iterator it = dead_cats_.begin(); it != dead_cats_.end(); ++it)
	{
		(*it)->SetProblem(false);
	}
}