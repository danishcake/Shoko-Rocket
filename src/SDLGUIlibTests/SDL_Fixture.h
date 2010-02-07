#pragma once
#include <sdl.h>
#include <iostream>
#include <Widget.h>

struct SDL_fixture
{
	bool SDL_init_ok;
	SDL_Surface* screen;
	SDL_fixture()
	{
		SDL_init_ok = true;
		int init_result;
		if((init_result = SDL_Init(SDL_INIT_VIDEO)) != 0)
		{
			SDL_init_ok = false;
			std::cout << "Error starting SDL\n" << init_result << "\n";
		}
		else
		{
			screen  = SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
			if(!screen)
			{
				SDL_init_ok = false;
				std::cout << "Error starting SDL\n";
			}else
			{
			//	std::cout << "Starting SDL\n";
			}
		}
	}

	~SDL_fixture()
	{
		if(SDL_init_ok)
		{
			SDL_Quit();
			//std::cout << "Shutting down SDL\n";
		}
		Widget::ClearRoot();
	}
};