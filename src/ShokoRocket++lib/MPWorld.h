#pragma once
#include "BaseWorld.h"

struct Spawner
{
public:
	Vector2i position;
	Direction::Enum direction;
};

struct PlayerRocket
{
public:
	Vector2i position;
	int player_id;
};

namespace PlayerArrowLevel
{
	enum Enum
	{
		FullArrow, HalfArrow, Clear
	};
}

struct PlayerArrow
{
public:
	Vector2i position;
	int player_id;
	bool halved;
	Direction::Enum direction;
};

class MPWorld : public BaseWorld
{
protected:
	vector<Spawner> spawners_;
	vector<PlayerRocket> player_rockets_; //Map might be more natural here
	vector<PlayerArrow> player_arrows_;

	unsigned int time_;
public:
	//Constructors
	MPWorld();
	MPWorld(string _filename);

	//Loading from XML
	virtual void LoadXML(TiXmlHandle& _root);

	//Update cat & mouse positions
	virtual WorldState::Enum Tick(float _dt);
	//Called when walkers reach a new gridsquare
	virtual void WalkerReachNewSquare(Walker* _walker);

	//Spawner adding etc
	void SetSpawner(Vector2i _position, Direction::Enum _direction);
	vector<Spawner>& GetSpawners(){return spawners_;}
	void ToggleSpawner(Vector2i _position, Direction::Enum _direction);

	//Player rocket adding etc
	void SetPlayerRocket(Vector2i _position, int _player_id);
	vector<PlayerRocket>& GetPlayerRockets(){return player_rockets_;}

	//Player arrows
	void SetPlayerArrow(Vector2i _position, Direction::Enum _direction, int player_id, PlayerArrowLevel::Enum _arrow_level);
	vector<PlayerArrow>& GetPlayerArrows(){return player_arrows_;}

	//Time
	unsigned int GetTime(){return time_;}

	//Walker state updates
	void UpdateCat(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time);
	void UpdateMouse(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time);
	void KillMouse(unsigned int _id, Vector2f _position, unsigned int _time);
	void KillCat(unsigned int _id, Vector2f _position, unsigned int _time);
	void CreateMouse(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time);
	void CreateCat(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time);

	void RemoveWalker(unsigned int _id, bool _kill, Vector2f _position, unsigned int _time);
	void UpdateWalker(unsigned int _id, Vector2f _position, Direction::Enum _direction, unsigned int _time);
};