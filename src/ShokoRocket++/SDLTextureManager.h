#pragma once
#include <TextureManager.h>
#include <map>
struct SDL_Surface;

class SDLTextureManager :
	public TextureManager
{
private:
	static unsigned int surface_flags_;
	static int depth_;

	std::map<std::string, SDL_Surface*> surface_cache_;

	virtual AnimationFrame* AcquireResource(Vector2i _offset, Vector2i _size, std::string _filename, float _time, Vector2i _frame_offset);
	virtual void InternalClearCache();
public:
};
