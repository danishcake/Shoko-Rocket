#include "SDLTextureManager.h"
#include "SDLAnimationFrame.h"
#include <SDL.h>
#include <SDL_image.h>
#include <boost/lexical_cast.hpp>
#include "Logger.h"

using std::string;

unsigned int SDLTextureManager::surface_flags_ = SDL_SWSURFACE | SDL_SRCALPHA;
int SDLTextureManager::depth_ = 32;

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine */
namespace
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 rmask = 0xff000000;
	Uint32 gmask = 0x00ff0000;
	Uint32 bmask = 0x0000ff00;
	Uint32 amask = 0x000000ff;
#else
	Uint32 rmask = 0x000000ff;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x00ff0000;
	Uint32 amask = 0xff000000;
#endif

	/* Copies data from RGBA with alpha */
	/* Otherwise when blitting the dest alpha is not changed, so can't copy subimage */
	void BlindBlit(SDL_Surface* _src, SDL_Rect* _offset, SDL_Surface* _dest)
	{
		int h_c = _src->h - _offset->y - _dest->h;
		int h = _dest->h + (h_c < 0 ? h_c : 0);
		int w_c = _src->w - _offset->x - _dest->w;
		int w = _dest->w + (w_c < 0 ? w_c : 0);
		int bpp = _src->format->BytesPerPixel;
		if(_src->format->BytesPerPixel != _dest->format->BytesPerPixel)
		{
			Logger::ErrorOut() << "Unable to Acquire resource, source data and dest data have different bpp\n";
			return;
		}

		//Copy _dest->h x _dest->w pixels from _src, starting at _offset
		//TODO fix if x+o.x > w
		for(int y = 0; y < h; y++)
		{
			char* dest = (char*)_dest->pixels + (bpp * y * _dest->w); //Start of dest row
			char* src = (char*)_src->pixels + (bpp * (_offset->y + y) * _src->w) + //Start of src row
											  (bpp * _offset->x);					 //Plus offset
			memcpy(dest, src, bpp * w);
		}
	}
}


AnimationFrame* SDLTextureManager::AcquireResource(Vector2i _offset, Vector2i _size, string _filename, float _time, Vector2i _frame_offset)
{
	static int sample_count = 0;
	SDL_Surface* whole_surface = IMG_Load(("Animations/" + _filename).c_str());
	/* This should ensure the BPP match */
	SDL_Surface* converted_whole_surface = SDL_DisplayFormatAlpha(whole_surface);
	SDL_FreeSurface(whole_surface);
	whole_surface = NULL;

	SDL_Surface* sampled_area = SDL_CreateRGBSurface(surface_flags_, _size.x, _size.y, depth_, rmask, gmask, bmask, amask);
	SDL_Surface* converted_sample = SDL_DisplayFormatAlpha(sampled_area);
	SDL_FreeSurface(sampled_area);
	sampled_area = NULL;
	
	// Blitting an opaque pixel to a transparent one results in a transparent pixel!
	sample_count++;
	SDL_Rect area;
	area.x = _offset.x;
	area.y = _offset.y;
	area.w = _size.x;
	area.h = _size.y;
	BlindBlit(converted_whole_surface, &area, converted_sample);
	
	
	SDL_FreeSurface(sampled_area);
	SDL_FreeSurface(whole_surface);
	SDLAnimationFrame* frame = new SDLAnimationFrame((int)(converted_sample), _time, _frame_offset, converted_sample);
	return frame;
}