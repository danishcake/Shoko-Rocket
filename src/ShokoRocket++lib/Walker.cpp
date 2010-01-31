#include "Walker.h"

#include <cmath>
#include "World.h"

int Walker::Total_created = 0;
int Walker::Total_destroyed = 0;


Walker::Walker(void)
{
	initial_direction_ = Direction::North;
	direction_ = initial_direction_;
	
	initial_position_ = Vector2f(0, 0);
	position_ = initial_position_;

	speed_ = 1.0f;
	walker_type_ = WalkerType::Mouse;

	pWorld_ = NULL;
	Total_created++;
	death_time_ = 0;
	walker_state_ = WalkerState::Live;
}

Walker::~Walker(void)
{
	Total_destroyed++;
}

bool Walker::DoTurns()
{
	if(pWorld_)
	{
		if(position_.x >= pWorld_->GetSize().x)
			position_.x -= pWorld_->GetSize().x;
		if(position_.y >= pWorld_->GetSize().y)
			position_.y -= pWorld_->GetSize().y;
		if(position_.y <= -1.0f)
			position_.y += pWorld_->GetSize().y;
		if(position_.x <= -1.0f)
			position_.x += pWorld_->GetSize().x;

		Direction::Enum old_direction = direction_;
		pWorld_->WalkerReachNewSquare(this);
		GridSquare gs = pWorld_->GetGridSquare(Vector2i(position_.x, position_.y));
		direction_ = gs.GetTurnDirection(direction_);
		return old_direction != direction_;
	}
	return false;
}

void Walker::Advance(float _timespan)
{
	float distance_to_go = speed_ * _timespan;
	while(distance_to_go > 0)
	{	
		float distance_to_next = 0;
		switch(direction_)
		{
			case Direction::Stopped:
				return;
			break;
			case Direction::North:
				distance_to_next = position_.y - floor(position_.y);
				if(distance_to_next == 0)
				{
					distance_to_next = 1.0f;
					if(DoTurns())
						continue;
				}

				if(distance_to_go >= distance_to_next)
				{
					distance_to_go -= distance_to_next;
					position_ += Vector2f( 0.0f, -1.0f) * distance_to_next;
					DoTurns();
				} else
				{
					position_ += Vector2f( 0.0f, -1.0f) * distance_to_go;
					distance_to_go = 0;
				}

				break;
			case Direction::South:
				distance_to_next = ceil(position_.y) - position_.y;
				if(distance_to_next == 0)
				{
					distance_to_next = 1.0f;
					if(DoTurns())
						continue;
				}

				if(distance_to_go >= distance_to_next)
				{
					distance_to_go -= distance_to_next;
					position_ += Vector2f( 0.0f,  1.0f) * distance_to_next;
					DoTurns();
				} else
				{
					position_ += Vector2f( 0.0f,  1.0f) * distance_to_go;
					distance_to_go = 0;
				}
				
				break;
			case Direction::East:
				distance_to_next = ceil(position_.x) - position_.x;
				if(distance_to_next == 0)
				{
					distance_to_next = 1.0f;
					if(DoTurns())
						continue;
				}

				if(distance_to_go >= distance_to_next)
				{
					distance_to_go -= distance_to_next;
					position_ += Vector2f( 1.0f,  0.0f) * distance_to_next;
					DoTurns();
				} else
				{
					position_ += Vector2f( 1.0f,  0.0f) * distance_to_go;
					distance_to_go = 0;
				}
				
				break;
			case Direction::West:
				distance_to_next = position_.x - floor(position_.x);
				if(distance_to_next == 0)
				{
					distance_to_next = 1.0f;
					if(DoTurns())
						continue;
				}
				
				if(distance_to_go >= distance_to_next)
				{
					distance_to_go -= distance_to_next;
					position_ += Vector2f(-1.0f,  0.0f) * distance_to_next;
					DoTurns();
				} else
				{
					position_ += Vector2f(-1.0f,  0.0f) * distance_to_go;
					distance_to_go = 0;
				}
				
				break;
		}
	}
}

void Walker::Reset()
{
	position_ = initial_position_;
	direction_ = initial_direction_;
	death_time_ = 0;
	walker_state_ = WalkerState::Live;
}

void Walker::SetWalkerType(WalkerType::Enum _walker_type)
{
	walker_type_ = _walker_type; 
	if(walker_type_ == WalkerType::Mouse)
		speed_ = 1.0f;
	if(walker_type_ == WalkerType::Cat)
		speed_ = 2.0f/3.0f;

}

void Walker::Kill()
{
	walker_state_ = WalkerState::Killed;
}

void Walker::Rescue()
{
	walker_state_ = WalkerState::Rescued;
}

WalkerState::Enum Walker::GetWalkerState()
{
	return walker_state_;
}