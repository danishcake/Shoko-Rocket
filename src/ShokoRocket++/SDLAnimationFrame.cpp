#include "SDLAnimationFrame.h"
#include <SDL.h>
#include <SDL_image.h>

SDL_Surface* SDLAnimationFrame::screen_ = NULL;

//Frame ID is actually a pointer to SDL surface
SDLAnimationFrame::SDLAnimationFrame(int _frame_id, float _time, Vector2i _frame_offset, SDL_Surface* _surface)
: AnimationFrame(_frame_id, _time, _frame_offset)
{
	surface_ = _surface;
}

SDLAnimationFrame::~SDLAnimationFrame(void)
{
	SDL_FreeSurface(surface_);
}

void SDLAnimationFrame::Draw(Vector2f _position)
{
	SDL_Rect dest_rect;
	dest_rect.x = static_cast<Sint16>(_position.x - offset_.x);
	dest_rect.y = static_cast<Sint16>(_position.y - offset_.y);
	SDL_BlitSurface(surface_, NULL, screen_, &dest_rect);
}