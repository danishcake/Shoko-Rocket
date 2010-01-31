#include "AnimationSet.h"
#include "Animation.h"

AnimationSet::AnimationSet(void)
{
	default_animation_ = NULL;
}

AnimationSet::~AnimationSet(void)
{
}


void AnimationSet::AddAnimation(Animation* _animation)
{
	if(default_animation_ == NULL)
		default_animation_ = _animation;
	animations_[_animation->GetName()] = _animation;
}

Animation* AnimationSet::GetAnimation(string _name)
{
	if(animations_.find(_name) != animations_.end())
		return animations_[_name];
	else
		return NULL;
}