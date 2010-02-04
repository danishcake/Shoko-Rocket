#include "Logger.h"
#include "BlittableRect.h"
#include <SDL.h>
#include <SDL_image.h>


unsigned int BlittableRect::surface_flags_ = SDL_SWSURFACE | SDL_SRCALPHA;
int BlittableRect::depth_ = 32;
unsigned int BlittableRect::bytes_used = 0;

SDL_Surface* font = NULL;

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
	void BlindBlit(SDL_Surface* _src, SDL_Surface* _dest, SDL_Rect* _offset)
	{
		SDL_LockSurface(_src);
		SDL_LockSurface(_dest);
		//Clamp height and width
		int w = _src->w + _offset->x < _dest->w ? _src->w : _dest->w - _offset->x;
		w = w < 0 ? 0 : w;
		int h = _src->h + _offset->y < _dest->h ? _src->h : _dest->h - _offset->y;
		h = h < 0 ? 0 : h;
		int bpp = _src->format->BytesPerPixel;

		if(_src->format->BytesPerPixel != _dest->format->BytesPerPixel)
		{
			Logger::ErrorOut() << "Unable to Acquire resource, source data and dest data have different bpp\n";
			return;
		}

		//Copy w x h pixels from _src to _dest
		//TODO fix negative offset
		for(int y = 0; y < h; y++)
		{
			char* dest = (char*)_dest->pixels + (bpp * (_offset->y + y) * _dest->w) +	//Start of dest row
												(bpp * _offset->x);						//Plus offset
			char* src = (char*)_src->pixels + (bpp * y * _src->w);//Start of src row
			memcpy(dest, src, bpp * w);
		}
		SDL_UnlockSurface(_src);
		SDL_UnlockSurface(_dest);
	}

	
	void SampleSurface(SDL_Surface* _src, SDL_Surface* _dest)
	{
		SDL_LockSurface(_src);
		SDL_LockSurface(_dest);
		int bpp = _src->format->BytesPerPixel;
		if(_src->format->BytesPerPixel != _dest->format->BytesPerPixel)
		{
			Logger::ErrorOut() << "Unable to Acquire resource, source data and dest data have different bpp\n";
			return;
		}
		for(int y = 0; y < _dest->h; y++)
		{
			for(int x = 0; x < _dest->w; x++)
			{
				int sample_x = static_cast<int>(((float)_src->w / (float)_dest->w) * x);
				int sample_y = static_cast<int>(((float)_src->h / (float)_dest->h) * y);
				char* dest = (char*)_dest->pixels + (_dest->pitch * y) +
													(bpp * x);		
				char* src = (char*)_src->pixels + (_src->pitch * sample_y) +
												  (bpp * sample_x);//Start of src row
				
				for(int b = 0; b < bpp; b++)
				{
					*dest = *src;
					dest++;
					src++;
				}
			}
		}
		SDL_UnlockSurface(_src);
		SDL_UnlockSurface(_dest);
	}
}


BlittableRect::BlittableRect(SDL_Surface* _surface, bool _dont_free_surface)
: size_(Vector2i(_surface->w, _surface->h)), surface_(_surface), dont_free_(_dont_free_surface), error_occurred_(false)
{
	bytes_used += surface_->w * _surface->h * _surface->format->BytesPerPixel;
}


BlittableRect::BlittableRect(Vector2i _size)
	: size_(_size)
{
	error_occurred_ = false;
	dont_free_ = false;
	surface_ = SDL_CreateRGBSurface(surface_flags_, _size.x, _size.y, depth_, rmask, gmask, bmask, amask);
	if(!surface_)
	{
		error_occurred_ = true;
		Logger::ErrorOut() << "Unable to create empty image of size " << _size << "\n";
	}
	SDL_Surface* conv_surface = SDL_DisplayFormatAlpha(surface_);
	SDL_FreeSurface(surface_);
	surface_ = conv_surface;
	SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, 0,0,0,255));
	bytes_used += surface_->w * surface_->h * surface_->format->BytesPerPixel;
}

BlittableRect::BlittableRect(std::string _filename)
{
	error_occurred_ = false;
	dont_free_ = false;
	surface_ = IMG_Load(("Animations/" + _filename).c_str());
	if(!surface_)
	{
		error_occurred_ = true;
		Logger::ErrorOut() << "Unable to load image " << _filename << "\n";
		return; //Oh shit, TODO errors here
	}
	SDL_Surface* conv_surface = SDL_DisplayFormatAlpha(surface_);
	SDL_FreeSurface(surface_);
	surface_ = conv_surface;
	size_ = Vector2i(surface_->w, surface_->h);
	bytes_used += surface_->w * surface_->h * surface_->format->BytesPerPixel;
}

BlittableRect::BlittableRect(std::string _filename, bool _dont_append_animations)
{
	error_occurred_ = false;
	dont_free_ = false;
	if(_dont_append_animations)
		surface_ = IMG_Load(_filename.c_str());
	else
		surface_ = IMG_Load(("Animations/" + _filename).c_str());
	if(!surface_)
	{
		error_occurred_ = true;
		Logger::ErrorOut() << "Unable to load image " << _filename << "\n";
		return; //Oh shit, TODO errors here
	}
	SDL_Surface* conv_surface = SDL_DisplayFormatAlpha(surface_);
	SDL_FreeSurface(surface_);
	surface_ = conv_surface;
	size_ = Vector2i(surface_->w, surface_->h);
	bytes_used += surface_->w * surface_->h * surface_->format->BytesPerPixel;
}

BlittableRect::BlittableRect(std::string _filename, Vector2i _size) 
: size_(_size)
{
	error_occurred_ = false;
	dont_free_ = false;
	surface_ = SDL_CreateRGBSurface(surface_flags_, _size.x, _size.y, depth_, rmask, gmask, bmask, amask);
	if(!surface_)
	{
		error_occurred_ = true;
		Logger::ErrorOut() << "Unable to create empty image of size " << _size << "\n";
		return; //Oh shit, TODO errors here
	}
	SDL_Surface* file_surface = IMG_Load(_filename.c_str());
	if(!file_surface)
	{
		error_occurred_ = true;
		Logger::ErrorOut() << "Unable to load image " << _filename << "\n";		
		return; //Oh shit, TODO errors here
	}
	SDL_BlitSurface(file_surface, NULL, surface_, NULL);
	SDL_FreeSurface(file_surface);


	SDL_Surface* conv_surface = SDL_DisplayFormatAlpha(surface_);
	SDL_FreeSurface(surface_);
	surface_ = conv_surface;
	bytes_used += surface_->w * surface_->h * surface_->format->BytesPerPixel;
}


BlittableRect::~BlittableRect(void)
{
	if(error_occurred_ == false && dont_free_ == false)
	{
		bytes_used -= surface_->w * surface_->h * surface_->format->BytesPerPixel;
		//Logger::DiagnosticOut() << "Surface memory used: " << (int)bytes_used << "\n";
		SDL_FreeSurface(surface_);
	} else
	{
		Logger::DiagnosticOut() << "Not freeing: Surface memory used: " << (int)bytes_used << "\n";
	}
}

void BlittableRect::Blit(Vector2i _position, BlittableRect* _dest)
{
	SDL_Rect dst_rect;
	dst_rect.x = static_cast<Sint16>(_position.x);
	dst_rect.y = static_cast<Sint16>(_position.y);
	dst_rect.w = static_cast<Uint16>(size_.x);
	dst_rect.h = static_cast<Uint16>(size_.y);
	SDL_BlitSurface(surface_, NULL, _dest->surface_, &dst_rect);
}

void BlittableRect::RawBlit(Vector2i _position, BlittableRect* _dest)
{
	SDL_Rect dst_rect;
	dst_rect.x = static_cast<Sint16>(_position.x);
	dst_rect.y = static_cast<Sint16>(_position.y);
	dst_rect.w = static_cast<Uint16>(size_.x);
	dst_rect.h = static_cast<Uint16>(size_.y);
	BlindBlit(surface_, _dest->surface_, &dst_rect);	
}

void BlittableRect::Fade(float _degree, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_Surface* fade_surface = SDL_CreateRGBSurface(surface_flags_, size_.x, size_.y, 32, rmask, gmask, bmask, amask);
	_degree = _degree < 0 ? 0 : (_degree > 1.0f ? 1.0f : _degree); //Limit to +-1.0f
	unsigned char a = static_cast<unsigned char>(255 * _degree);
	SDL_FillRect(fade_surface, NULL, SDL_MapRGBA(fade_surface->format, r, g, b, a));
	
	SDL_BlitSurface(fade_surface, NULL, surface_, NULL);
	SDL_FreeSurface(fade_surface);
}

void BlittableRect::Fill(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, r, g, b, a));
}

void BlittableRect::BlitText(std::string _text, TextAlignment::Enum _alignment)
{
	if(!font)
	{
		font = IMG_Load("Animations/Font.png");
	}
	if(!font || font->w != 256 || font->h != 144)
	{
		//TODO error
		Logger::ErrorOut() << "Unable to font image  Animation/Font.png\n";
		return;
	}
	int out_x = 0;
	int out_y = 0;
	const int font_width = 16;
	const int font_height = 24;
	switch(_alignment)
	{
	case TextAlignment::TopLeft:
		out_x = 4;
		out_y = 4;
		break;
	case TextAlignment::Top:
		out_x = (size_.x/2) - static_cast<int>(_text.length()) * (font_width / 2);
		out_y = 4;
		break;
	case TextAlignment::TopRight:
		out_x = size_.x - static_cast<int>(_text.length()) * font_width - 4;
		out_y = 4;
		break;
	case TextAlignment::Left:
		out_x = 4;
		out_y = (size_.y / 2) - (font_height / 2);
		break;
	case TextAlignment::Centre:
		out_x = (size_.x/2) - static_cast<int>(_text.length()) * (font_width / 2);
		out_y = (size_.y / 2) - (font_height / 2);
		break;
	case TextAlignment::Right:
		out_x = size_.x - static_cast<int>(_text.length()) * font_width - 4;
		out_y = (size_.y / 2) - (font_height / 2);
		break;
	case TextAlignment::BottomLeft:
		out_x = 4;
		out_y = size_.y - font_height - 4;
		break;
	case TextAlignment::Bottom:
		out_x = (size_.x/2) - static_cast<int>(_text.length()) * (font_width / 2);
		out_y = size_.y - font_height - 4;
		break;
	case TextAlignment::BottomRight:
		out_x = size_.x - static_cast<int>(_text.length()) * font_width - 4;
		out_y = size_.y - font_height - 4;
		break;
	}
	for(unsigned int i = 0; i < _text.length(); i++)
	{
		unsigned char c = _text.c_str()[i];
		c -= 32;
		if(c >= 96)
			continue;
		int sample_x = (c % 16) * font_width;
		int sample_y = (c / 16) * font_height;
		SDL_Rect src_rect;
		src_rect.x = static_cast<Sint16>(sample_x);
		src_rect.y = static_cast<Sint16>(sample_y);
		src_rect.w = font_width;
		src_rect.h = font_height;


		SDL_Rect dest_rect;
		dest_rect.x = static_cast<Sint16>(out_x);
		dest_rect.y = static_cast<Sint16>(out_y);
		dest_rect.w = font_width;
		dest_rect.h = font_height;
		SDL_BlitSurface(font, &src_rect, surface_, &dest_rect);
		out_x += 16;
	}
}

BlittableRect* BlittableRect::Resize(Vector2i _new_size)
{
	BlittableRect* sample = new BlittableRect(_new_size);
	SampleSurface(surface_, sample->surface_);
	return sample;
}

void BlittableRect::Save(std::string _filename)
{
	SDL_SaveBMP(surface_, _filename.c_str());
}

void BlittableRect::SetAlpha(unsigned char a)
{
	SDL_SetAlpha(surface_, 0, a);
}