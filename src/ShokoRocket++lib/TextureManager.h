#pragma once
#include <map>
#include <string>
#include "vmath.h"

class Animation;
class AnimationFrame;
class AnimationSet;

class TextureManager
{
private:
	std::map<std::string, AnimationSet*> animations_;
	static TextureManager* instance_;
	static TextureManager* GetInstance();
	static AnimationSet* AddAnimationSet(std::string _xml_animation_set);
public:
	
	static AnimationSet* GetAnimationSet(std::string _xml_animation_set);
	static Animation* GetAnimation(std::string _xml_animation);

	static void SetTextureManager(TextureManager* _instance){instance_ = _instance;}
	static void Release();
	/* To be overriden in implementing classes (SDL/OpenGL/DirectX) */
	/* Should return an ID that can be used to find resource */
	virtual AnimationFrame* AcquireResource(Vector2i _offset, Vector2i _size, std::string _filename, float _time, Vector2i _frame_offset);
};