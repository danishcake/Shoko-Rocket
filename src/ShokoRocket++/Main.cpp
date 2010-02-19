#include "SDLTextureManager.h"
#include "SDLAnimationFrame.h"
#include <tchar.h>
#include <boost/lexical_cast.hpp>
#include <sdl.h>
#include <Logger.h>
#include "Settings.h"
#include <ctime>
#include "GameStateMachine.h"
#include "StandardTextures.h"
#include "Widget.h"

SDL_Surface* p_screen = NULL;
BlittableRect* p_screen_rect = NULL;
BlittableRect* p_background = NULL;
GameStateMachine* gsm = NULL;
clock_t ltv_time;

SDL_Surface* SDL_init()
{
	SDL_Init(SDL_INIT_VIDEO);
	Vector2i resolution = Settings::GetResolution();
	SDL_Surface* p_surface = SDL_SetVideoMode(resolution.x, resolution.y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | (Settings::GetFullScreen() ? SDL_FULLSCREEN : 0));
	if(!p_surface)
	{
		Logger::ErrorOut() << "Unable to create screen surface, aborting\n";
	} else
	{
		Logger::DiagnosticOut() << "Screen surface created\n" <<
								   "Resolution = " << p_surface->w << "," << p_surface->h << "\n" <<
								   "Bitdepth = " << p_surface->format->BitsPerPixel <<"\n" <<
								   "Hardware = "  << (p_surface->flags & SDL_HWSURFACE ? "true" : "false") << "\n" <<
								   "Doublebuffered = " << (p_surface->flags & SDL_DOUBLEBUF ? "true" : "false") << "\n" <<
								   "Fullscreen = " << (p_surface->flags & SDL_FULLSCREEN ? "true" : "false") << "\n";
				
		p_screen_rect = new BlittableRect(p_surface, false);
	}
	
	return p_surface;
}

bool AcquireResources()
{
	if(Settings::GetResolution() == Vector2i(640, 480))
	{
		p_background = new BlittableRect("Background640x480.png");
	} else if(Settings::GetResolution() == Vector2i(800, 600))
	{
		p_background = new BlittableRect("Background800x600.png");
	} else if(Settings::GetResolution() == Vector2i(1024, 768))
	{
		p_background = new BlittableRect("Background1024x768.png");
	} else if(Settings::GetResolution() == Vector2i(1280, 1024))
	{
		p_background = new BlittableRect("Background1280x1024.png");
	} else if(Settings::GetResolution() == Vector2i(1280, 960))
	{
		p_background = new BlittableRect("Background1280x960.png");
	} else if(Settings::GetResolution() == Vector2i(1280, 768))
	{
		p_background = new BlittableRect("Background1280x768.png");
	} else if(Settings::GetResolution() == Vector2i(1024, 600))
	{
		p_background = new BlittableRect("Background1024x600.png");
	} else if(Settings::GetResolution() == Vector2i(1600, 1200))
	{
		p_background = new BlittableRect("Background1600x1200.png");
	} else
	{
		BlittableRect br("BackgroundSolid.png");
		BlittableRect br2("BackgroundTile.png");

		p_background = new BlittableRect(Settings::GetResolution());
		for(int x = 0; x < Settings::GetResolution().x / 160 + 1; x++)
		{
			for(int y = 0; y < Settings::GetResolution().y / 160 + 1; y++)
			{
				br.RawBlit(Vector2i(x * 160, y * 160), p_background);
			}
		}
		int xoffset = (Settings::GetResolution().x % 160) / 2;
		int yoffset = (Settings::GetResolution().y % 160) / 2;
		for(int x = 0; x < Settings::GetResolution().x / 160; x++)
		{
			for(int y = 0; y < Settings::GetResolution().y / 160; y++)
			{
				br2.RawBlit(Vector2i(xoffset + x * 160, yoffset + y * 160), p_background);
			}
		}
	}
	if(!p_background)
	{
		Logger::ErrorOut() << "Unabled to load Background.png\n";
	}
	return true;
}

bool GameTick()
{
	clock_t current_time = clock();
	float dt = (float)(current_time - ltv_time) / (float)CLOCKS_PER_SEC;
	ltv_time = current_time;
	if(dt > 0)
	{
		int fps = static_cast<int>(1.0f / dt);
		SDL_WM_SetCaption((boost::lexical_cast<std::string, int>(fps)).c_str(), "");
	}
	static int overrun = 0;
	if(dt > 0.1f)
	{
		if(!overrun)
			Logger::DiagnosticOut() << "Frame overrun: " << dt << "s, reduced to 0.1s\n";
		overrun++;
		dt = 0.1f;
	} else
	{
		if(overrun)
			Logger::DiagnosticOut() << "On time frame after " << overrun << " overrun frames\n";
		overrun = 0;
	}
	StandardTextures::TickAnimations(dt);
	Widget::Tick(dt);
	return gsm->Tick(dt);
}

void Draw()
{
	SDL_FillRect(p_screen, NULL, SDL_MapRGB(p_screen->format, 0, 0, 0));
	p_background->Blit(Vector2i(0, 0), p_screen_rect);
	gsm->Draw(p_screen);
	Widget::RenderRoot(p_screen_rect);
	SDL_Flip(p_screen);
}

int _tmain(int /*argc*/, _TCHAR* /*argv*/[])
{
	bool bFinished = false;
	p_screen = SDL_init();
	ltv_time = clock();
	
	if(p_screen)
	{
		SDLTextureManager::SetTextureManager(new SDLTextureManager());
		SDLAnimationFrame::screen_ = p_screen;
		bFinished = (AcquireResources() == false);
		StandardTextures::LoadTextures();
		gsm = new GameStateMachine();
	}
	


	while(!bFinished)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				bFinished = true;
				break;
			}
			Widget::DistributeSDLEvents(&event);
		}
		
		bFinished |= GameTick();
		Draw();

		SDL_Delay(10);
	}
	delete gsm;
	SDL_Quit();
	return 0;
}