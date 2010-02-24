#pragma once
#include "vmath.h"
#include "Direction.h"

class BaseWorld;

namespace WalkerType
{
	enum Enum
	{
		Mouse, Cat
	};
}

namespace WalkerState
{
	enum Enum
	{
		Live, Killed, Rescued
	};
}

class Walker
{
protected:
	Vector2f position_;
	Vector2f initial_position_;
	Direction::Enum direction_;
	Direction::Enum initial_direction_;
	float speed_;
	BaseWorld* pWorld_;
	WalkerType::Enum walker_type_;
	WalkerState::Enum walker_state_;
	float death_time_;
	bool problem_;

public:
	Walker(void);
	~Walker(void);
	static int Total_created;
	static int Total_destroyed;

	/* Gets and sets the position */
	Vector2f GetPosition(){return position_;}
	void SetPosition(Vector2f _position){initial_position_ = _position; position_ = _position;}
	/* Gets and sets the direction */
	Direction::Enum GetDirection(){return direction_;}
	void SetDirection(Direction::Enum _direction){initial_direction_ = _direction; direction_ = _direction;}
	/* Reacts to an arrow on the world */
	void EncounterArrow(Direction::Enum _direction){direction_ = _direction;}
	/* Gets and sets the walker type */
	WalkerType::Enum GetWalkerType(){return walker_type_;}
	void SetWalkerType(WalkerType::Enum _walker_type);

	/* Gets and sets the world */
	BaseWorld* GetWorld(){return pWorld_;}
	void SetWorld(BaseWorld* _pWorld){pWorld_ = _pWorld;}


	
	/* Causes the walker to advance for a number of seconds */
	void Advance(float _timespan);
	/* Causes the walker to examine environment, and if finding it's route blocked it will turn */
	bool DoTurns();
	/* Resets the walker to the initial position */
	void Reset();
	/* Counts the time spent dead for the walker */
	void DeathTick(float _timespan){death_time_ += _timespan;};
	float GetDeathTime(){return death_time_;}
	
	/* Changes the walker state to dead */
	void Kill();
	/* Changes the walker state to rescued */
	void Rescue();
	WalkerState::Enum GetWalkerState();

	/* The entity can be marked as a problem if it collided */
	void SetProblem(bool _problem){problem_ = _problem;}
	bool GetProblem(){return problem_;}
};
