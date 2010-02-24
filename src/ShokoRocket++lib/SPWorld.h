#pragma once
#include "BaseWorld.h"


struct ArrowRecord
{
	Vector2i Position;
	Direction::Enum Direction;
};

class SPWorld : 
	public BaseWorld
{
protected:
	vector<Vector2f> problem_points_;

	vector<Direction::Enum> arrows_;
	vector<ArrowRecord> solution_arrows_;
	bool unlimited_arrow_stock_;

	int rescued_mice_;

	//Loading from XML
	virtual void LoadXML(TiXmlHandle& _root);
public:
	SPWorld(void);
	SPWorld(std::string _filename);

	//Advances all the mice and cats and collides everything
	virtual WorldState::Enum Tick(float _dt);
	//Resets the level to it's starting state(Mice reset, placed arrows restored)
	void Reset();
	//Saves the level to XML
	void SaveAs(string _filename);

	//Adds and queries mice
	int GetRescuedMice(){return rescued_mice_;}
	void RescueAllMice();
	void KillAllMice();

	//Arrow helper functions and stock querier
	void AddArrow(Direction::Enum _arrow){arrows_.push_back(_arrow);}
	vector<Direction::Enum> GetArrows(){return arrows_;}
	void ClearArrow(Vector2i _position);
	void ToggleArrow(Vector2i _position, Direction::Enum _direction);
	void ToggleNorthArrow(Vector2i _position){ToggleArrow(_position, Direction::North);}
	void ToggleSouthArrow(Vector2i _position){ToggleArrow(_position, Direction::South);}
	void ToggleEastArrow(Vector2i _position){ToggleArrow(_position, Direction::East);}
	void ToggleWestArrow(Vector2i _position){ToggleArrow(_position, Direction::West);}
	void ClearArrows();
	void SetArrowStockUnlimited(){unlimited_arrow_stock_ = true;}


	void LoadSolution();
	vector<Vector2f> GetProblemPoints(){return problem_points_;}
	void ResetProblemWalkers();

	virtual void WalkerReachNewSquare(Walker* _walker);

};
