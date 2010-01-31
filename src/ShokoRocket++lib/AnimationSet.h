#pragma once
#include <map>
#include <string>
using std::map;
using std::string;

class Animation;

class AnimationSet
{
private:
	map<string, Animation*> animations_;
	Animation* default_animation_;
public:
	AnimationSet(void);
	~AnimationSet(void);

	void AddAnimation(Animation* _animation);
	Animation* GetAnimation(string _name);
	Animation* GetDefaultAnimation(){return default_animation_;}

	int GetAnimationCount(){return animations_.size();}
};
