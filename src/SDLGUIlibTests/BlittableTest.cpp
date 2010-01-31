#include "stdafx.h"
#include <BlittableRect.h>
#include <sdl.h>

TEST_FIXTURE(SDL_fixture, BlittableLoadsPNG)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		BlittableRect parent(Vector2i(128,128));
		BlittableRect childA("Missing.png");
		BlittableRect childB("Present.png");
		childB.Blit(Vector2i(32,32), &parent); 
		CHECK_EQUAL(false, parent.GetError());
		CHECK_EQUAL(true, childA.GetError());
		CHECK_EQUAL(false, childB.GetError());
		CHECK_EQUAL(Vector2i(128, 128), childB.GetSize());
	}
} 

TEST_FIXTURE(SDL_fixture, BlindBlitting)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		BlittableRect root(Vector2i(128, 128));
		BlittableRect childA(Vector2i(128, 128));
		CHECK_EQUAL(false, root.GetError());
		CHECK_EQUAL(false, childA.GetError());
		childA.RawBlit(Vector2i(64, 64), &root);
	}
}
