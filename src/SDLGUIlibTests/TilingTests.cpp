#include "stdafx.h"
#include <Widget.h>

TEST_FIXTURE(SDL_fixture, VerticalTile)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		VerticalTile vt("Top.png", "Middle.png", "Bottom.png");
		CHECK_EQUAL("Top.png", vt.top);
		CHECK_EQUAL("Middle.png", vt.middle);
		CHECK_EQUAL("Bottom.png", vt.bottom);
		CHECK_EQUAL(TilingType::Vertical, vt.type);

		Widget* tiled = new Widget(vt, 100);
	}
}

TEST_FIXTURE(SDL_fixture, HorizontalTile)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		HorizontalTile ht("Left.png", "Middle.png", "Right.png");
		CHECK_EQUAL("Left.png", ht.left);
		CHECK_EQUAL("Middle.png", ht.middle);
		CHECK_EQUAL("Right.png", ht.right);
		CHECK_EQUAL(TilingType::Horizontal, ht.type);

		Widget* tiled = new Widget(ht, 100);
	}
}

TEST_FIXTURE(SDL_fixture, RectangularTile)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
	}
}

TEST_FIXTURE(SDL_fixture, MissingFilesReportError)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
	}
}
