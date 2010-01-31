#include "stdafx.h"
#include "Widget.h"

TEST_FIXTURE(SDL_fixture, TextEditting)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Event e;
		e.event_type = EventType::MouseDown;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event.mouse_event.x = 5;
		e.event.mouse_event.y = 5;
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		p_widget->HandleEvent(e);
		CHECK_EQUAL(true, p_widget->HasEditting());
		CHECK_EQUAL(true, p_widget->HasModal());


	}
}

