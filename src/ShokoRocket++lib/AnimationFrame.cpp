#include "AnimationFrame.h"

AnimationFrame::AnimationFrame(int _frame_id, float _time, Vector2i _offset)
{
	frame_id_ = _frame_id;
	time_to_next_ = _time;
	offset_ = _offset;
}

AnimationFrame::~AnimationFrame()
{
}