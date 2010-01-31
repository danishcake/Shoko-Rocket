#pragma once
#include <TextureManager.h>

class SDLTextureManager :
	public TextureManager
{
private:
	static unsigned int surface_flags_;
	static int depth_;
public:
	virtual AnimationFrame* AcquireResource(Vector2i _offset, Vector2i _size, std::string _filename, float _time, Vector2i _frame_offset);
};
