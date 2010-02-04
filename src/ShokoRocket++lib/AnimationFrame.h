#pragma once
#include "vmath.h"

class AnimationFrame
{
protected:
	Vector2i offset_;
private:
	int frame_id_;
	float time_to_next_;

public:
	AnimationFrame(int _frame_id, float _time, Vector2i _offset);
	virtual ~AnimationFrame();
	int GetFrameID(){return frame_id_;}
	float GetTime(){return time_to_next_;}
	Vector2i GetOffset(){return offset_;}
	virtual void Draw(Vector2f /*_position*/){} //To be overridden
	
};
