#include "stdafx.h"
#include "Widget.h"

TEST_FIXTURE(SDL_fixture, TextPrinting)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
	}
}

