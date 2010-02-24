#pragma once
#include "vmath.h"
#include "GridSquare.h"
#include <vector>

using std::vector;
using std::string;

namespace
{
	struct TopLeft
	{
		bool top;
		bool left;
	};
}

namespace SquareType
{
	enum Enum
	{
		Empty, Rocket, Hole, NorthArrow, SouthArrow, EastArrow, WestArrow, HalfNorthArrow, HalfSouthArrow, HalfEastArrow, HalfWestArrow, DestroyedNorthArrow, DestroyedSouthArrow, DestroyedEastArrow, DestroyedWestArrow
	};

	Direction::Enum GetDirection(Enum _square_type);
	Enum FromDirection(Direction::Enum _direction);
	Enum Diminish(Enum _square_type);
	Enum RestoreToFull(Enum _square_type);
}

namespace WorldState
{
	enum Enum
	{
		OK, FileLoadError, Victory, Defeat
	};
}

class Walker;
class TiXmlHandle;

class BaseWorld
{
protected:
	Vector2i size_;
	vector<vector<TopLeft>> walls_;
	vector<vector<SquareType::Enum>> special_squares_;
	vector<Walker*> mice_;
	vector<Walker*> cats_;
	vector<Walker*> dead_mice_;
	vector<Walker*> dead_cats_;
	vector<Walker*> just_dead_mice_;
	vector<Walker*> just_dead_cats_;

	string name_;
	string filename_;
	WorldState::Enum state_;

	virtual void LoadXML(TiXmlHandle& _root);
public:
	//Constructors
	BaseWorld();
	virtual ~BaseWorld(void);

	//Ticks dead walkers, extra functionality provided by subclasses
	virtual WorldState::Enum Tick(float _dt);

	//Gets / sets the size of the level
	Vector2i GetSize(){return size_;}
	void SetSize(Vector2i _size);
	//Gets / sets the walls around a coordinate
	GridSquare GetGridSquare(Vector2i _point);
	void SetGridSquare(Vector2i _point, GridSquare _gs);
	void ToggleWall(Vector2i _position, Direction::Enum _direction);

	//Gets / sets the square type
	SquareType::Enum GetSquareType(Vector2i _point);
	bool SetSquareType(Vector2i _point, SquareType::Enum _square_type);
	void ToggleRocket(Vector2i _position);
	void ToggleHole(Vector2i _position);

	//Gets / sets the name of the level
	string GetName(){return name_;}
	void SetName(string _name){name_ = _name;}

	string GetFilename(){return filename_;}

	//Adds and queries mice
	void AddMouse(Vector2i _position, Direction::Enum _direction);
	void AddMouse(Walker* _pMouse);
	vector<Walker*>& GetMice(){return mice_;}
	vector<Walker*>& GetDeadMice(){return dead_mice_;}
	int GetTotalMice(){return mice_.size() + dead_mice_.size();}
	void ToggleMouse(Vector2i _position, Direction::Enum _direction);

	//Adds and queries cats
	void AddCat(Vector2i _position, Direction::Enum _direction);
	void AddCat(Walker* _pCat);
	vector<Walker*>& GetCats(){return cats_;}
	vector<Walker*>& GetDeadCats(){return dead_cats_;}
	void ToggleCat(Vector2i _position, Direction::Enum _direction);

	//Called by walkers, to be overriden by base classes
	virtual void WalkerReachNewSquare(Walker* _walker) = 0;

	//Gets the error state
	bool GetError(){return state_ == WorldState::FileLoadError;}

	//Range calculations with wrapping
	static float GetShortestDistance(Vector2f _position1, Vector2f _position2, Vector2f _wrap_size);
};