#include "Animation.h"
#include "AnimationFrame.h"
#include <cmath>
#include <boost/lexical_cast.hpp>


int Animation::count_ = 0;

Animation::Animation()
{
	total_length_ = 0;
	frame_time_ = 0;
	current_frame_ = NULL;
	current_frame_index_ = 0;
	name_ = "Default" + boost::lexical_cast<string, int>(count_++);
}

Animation::~Animation()
{
	for(vector<AnimationFrame*>::iterator it = frames_.begin(); it != frames_.end(); ++it)
	{
		delete *it;
	}
}

void Animation::Tick(float _timespan)
{
	frame_time_ += _timespan;
	//Advance to next frame if time has elapsed
	while(frame_time_ > current_frame_->GetTime())
	{
		frame_time_ -= current_frame_->GetTime();
		current_frame_index_++;
		current_frame_index_ %= frames_.size();
		current_frame_ = frames_[current_frame_index_];
	}
}

AnimationFrame* Animation::GetFrame(float _time)
{
	_time = fmodf(_time, total_length_);
	float total_time_ = 0;
	for(vector<AnimationFrame*>::iterator it = frames_.begin(); it != frames_.end(); ++it)
	{
		total_time_ += (*it)->GetTime();
		if(total_time_ > _time)
			return *it;
	}
	return frames_[0];
}

AnimationFrame* Animation::GetFrameByIndex(int _index)
{
	return frames_[_index % frames_.size()];
}

int Animation::GetFrameID(float _time)
{
	return GetFrame(_time)->GetFrameID();
}

int Animation::GetCurrentFrameID()
{
	return current_frame_->GetFrameID();
}

void Animation::AddFrame(AnimationFrame* _frame)
{
	frames_.push_back(_frame);
	if(frames_.size() == 1) 
	{
		current_frame_ = _frame;
	}
	total_length_ += _frame->GetTime();
}