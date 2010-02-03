#include "stdafx.h"
#include "Widget.h"

/*
 * Checks that widgets correctly gain the editting state when clicked and editable
 */
TEST_FIXTURE(SDL_fixture, TextEditBasics)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Event e;
		e.event_type = EventType::MouseUp;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event.mouse_event.x = 5;
		e.event.mouse_event.y = 5;
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		p_widget->HandleEvent(e);
		CHECK_EQUAL(true, p_widget->HasEditting());
		
		//Escape cancels input
		e.event_type = EventType::KeyEscape;
		p_widget->HandleEvent(e);
		CHECK_EQUAL(false, p_widget->HasEditting());
	}
}

/*
 * Checks the editting widget is cleared on destruction
 */
TEST_FIXTURE(SDL_fixture, TextEditClears)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Event e;
		e.event_type = EventType::MouseUp;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event.mouse_event.x = 5;
		e.event.mouse_event.y = 5;
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		CHECK_EQUAL(true, p_widget->GetEditable());
		delete p_widget;

		CHECK_EQUAL((Widget*)NULL, Widget::GetEdittingWidget());

	}
}