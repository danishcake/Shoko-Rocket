#include "stdafx.h"
#include <Widget.h>
#include <sdl.h>

TEST_FIXTURE(SDL_fixture, WidgetRoot)
{
	Widget::ClearRoot();
	Widget* w = new Widget();
	Widget* w2 = new Widget();
	
	CHECK_EQUAL(2, Widget::GetRoot().size());
	w->AddChild(w2);
	CHECK_EQUAL(1, Widget::GetRoot().size());
	Widget::ClearRoot();
}

namespace
{
	int click_count = 0;
	void GetClick(Widget* /*_widget*/)
	{
		click_count++;
	}
}


TEST_FIXTURE(SDL_fixture, WidgetDistributeSDLEvents)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		click_count = 0;
		Widget* w = new Widget();
		w->SetPosition(Vector2i(20, 20));
		w->OnClick.connect(GetClick);
		
		CHECK_EQUAL(0, click_count);
		SDL_Event* pEvent = new SDL_Event();
		pEvent->type = SDL_MOUSEBUTTONUP;
		pEvent->button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(pEvent);
		CHECK_EQUAL(0, click_count);

		pEvent->button.x = 30;
		pEvent->button.y = 30;
		Widget::DistributeSDLEvents(pEvent);
		CHECK_EQUAL(1, click_count);

		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetsCanRejectFocus)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		click_count = 0;
		Widget* w = new Widget();
		w->SetRejectsFocus(false);

		SDL_Event* pEvent = new SDL_Event();
		pEvent->type = SDL_MOUSEBUTTONUP;
		pEvent->button.button = SDL_BUTTON_LEFT;
		pEvent->button.x = 30;
		pEvent->button.y = 30;
		w->OnClick.connect(GetClick);

		Widget::DistributeSDLEvents(pEvent);
		CHECK_EQUAL(1, click_count);
		//TODO work out how to actually test

		Widget::ClearRoot();
	}
}
