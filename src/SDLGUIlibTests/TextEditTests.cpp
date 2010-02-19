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

/*
 * Checks loss of focus removes editting
 */
TEST_FIXTURE(SDL_fixture, TextEditLossOfFocusEnds)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		Widget* p_widget2 = new Widget();
		p_widget2->SetPosition(Vector2i(128, 0));

		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		p_widget->SetEditting(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_BACKSPACE;
		e.event.key_event.key_up = true;
		p_widget->HandleEvent(e);
		
		CHECK_EQUAL("Howd", p_widget->GetText());
		
		
		e.event_type = EventType::MouseUp;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event.mouse_event.x = 10;
		e.event.mouse_event.y = 10;

		p_widget2->HandleEvent(e);
		
		
		CHECK_EQUAL(false, p_widget->HasEditting());
	}
}

/*
 * Checks characters appendable
 */
TEST_FIXTURE(SDL_fixture, TextEditAppends)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		p_widget->SetEditting(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_a;
		e.event.key_event.shift = false;
		e.event.key_event.key_up = true;
		p_widget->HandleEvent(e);
		CHECK_EQUAL("Howdya", p_widget->GetText());
		e.event.key_event.key_code = SDLK_b;
		e.event.key_event.shift = true;
		p_widget->HandleEvent(e);
		CHECK_EQUAL("HowdyaB", p_widget->GetText());
	}
}

/*
 * Checks the characters deletable
 */
TEST_FIXTURE(SDL_fixture, TextEditDeletes)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		p_widget->SetEditting(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_BACKSPACE;
		e.event.key_event.key_up = true;
		p_widget->HandleEvent(e);
		CHECK_EQUAL("Howd", p_widget->GetText());
		p_widget->HandleEvent(e);
		p_widget->HandleEvent(e);
		p_widget->HandleEvent(e);
		CHECK_EQUAL("H", p_widget->GetText());
		p_widget->HandleEvent(e);
		CHECK_EQUAL("", p_widget->GetText());
		p_widget->HandleEvent(e);
		CHECK_EQUAL("", p_widget->GetText());
	}
}

/*
 * Checks that enter ends editting
 */
TEST_FIXTURE(SDL_fixture, TextEditEnterEndsEditting)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		p_widget->SetEditting(true);
		CHECK_EQUAL(true, p_widget->GetEditable());

		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_BACKSPACE;
		e.event.key_event.key_up = true;
		p_widget->HandleEvent(e);
		CHECK_EQUAL("Howd", p_widget->GetText());
		e.event_type = EventType::KeyEnter;
		p_widget->HandleEvent(e);
		CHECK_EQUAL(false, p_widget->HasEditting());
	}
}

/*
 * Checks that editting has no effect if not in edit mode
 */
TEST_FIXTURE(SDL_fixture, TextEditRequiresEditMode)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widget = new Widget();
		p_widget->SetText("Howdy", TextAlignment::TopLeft);
		p_widget->SetEditable(true);
		p_widget->SetEditting(false);

		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_BACKSPACE;
		p_widget->HandleEvent(e);
		CHECK_EQUAL("Howdy", p_widget->GetText());

		p_widget->SetEditable(false);
		p_widget->SetEditting(true);

		p_widget->HandleEvent(e);
		CHECK_EQUAL("Howdy", p_widget->GetText());
	}
}