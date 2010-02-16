#include "stdafx.h"
#include <Widget.h>

TEST_FIXTURE(SDL_fixture, WidgetDefaults)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* w = new Widget();
		CHECK_EQUAL(Vector2i(0,0), w->GetPosition());
		CHECK_EQUAL(Vector2i(128,48), w->GetSize());
		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetSetters)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* w = new Widget();
		w->SetSize(Vector2i(50,60));
		w->SetPosition(Vector2i(5,5));
		CHECK_EQUAL(Vector2i(50, 60), w->GetSize());
		CHECK_EQUAL(Vector2i(5, 5), w->GetPosition());
		Widget::ClearRoot();
	}
}

