#pragma once
#include "AnimationFrame.h"

struct SDL_Surface;

class SDLAnimationFrame :
	public AnimationFrame
{
private:
	SDL_Surface* surface_;
public:
	SDLAnimationFrame(int _frame_id, float _time, Vector2i _frame_offset, SDL_Surface* _surface);
	virtual ~SDLAnimationFrame(void);

	static SDL_Surface* screen_;
	virtual void Draw(Vector2f _position);
};
