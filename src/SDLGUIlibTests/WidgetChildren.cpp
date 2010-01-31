#include "stdafx.h"
#include <Widget.h>
#include <algorithm>

TEST_FIXTURE(SDL_fixture, WidgetChildren)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* w = new Widget();
		Widget* w2 = new Widget();
		w->AddChild(w2);
		CHECK(std::find(w->GetChildren().begin(), w->GetChildren().end(), w2) != w->GetChildren().end());
		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, AddAndRemove)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* w = new Widget();
		Widget* w2 = new Widget();
		Widget* w3 = new Widget();
		w->AddChild(w2);
		w->AddChild(w3);
		CHECK_EQUAL(2, w->GetChildren().size());
		w->RemoveChild(w2);
		CHECK_EQUAL(1, w->GetChildren().size());
		Widget::ClearRoot();
	}
}

