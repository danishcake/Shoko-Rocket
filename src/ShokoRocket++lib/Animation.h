#pragma once
#include <vector>
#include <string>
using std::vector;
using std::string;

class AnimationFrame;

class Animation
{
private:
	float total_length_;

	float frame_time_;
	AnimationFrame* current_frame_;
	int current_frame_index_;

	vector<AnimationFrame*> frames_;
	string name_;
	static int count_;
public:
	Animation();
	~Animation();

	/* Directly access a frame by time */
	AnimationFrame* GetFrame(float _time);
	int GetFrameID(float _time);

	/* Add a frame to animation */
	void AddFrame(AnimationFrame* _frame);

	/* Get the current frame. Animation stores time */
	AnimationFrame* GetCurrentFrame(){return current_frame_;}
	/* Gets the frame at this index. If index out of bounds then returns frame at mod of _index */
	AnimationFrame* GetFrameByIndex(int _index);

	int GetCurrentFrameID();
	void Tick(float _timespan);
	string GetName(){return name_;}
	void SetName(string _name){name_ = _name;}
};
