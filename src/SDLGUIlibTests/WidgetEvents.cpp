#include "stdafx.h"
#include <Widget.h>
#include <Event.h>

namespace
{
	int parent_click_count = 0;
	void parent_callback(Widget* /*_widget*/)
	{
		parent_click_count++;
	}

	int child_click_count = 0;
	void child_callback(Widget* /*_widget*/)
	{
		child_click_count++;
	}
}

TEST_FIXTURE(SDL_fixture, WidgetClickFiresCallback)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		parent_click_count = 0;
		Event e;
		e.event.mouse_event.x = 10;
		e.event.mouse_event.y = 10;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event_type = EventType::MouseUp;
		
		Widget* w = new Widget();

		boost::signals::scoped_connection c = w->OnClick.connect(parent_callback);
		w->HandleEvent(e);
		CHECK_EQUAL(1, parent_click_count);
		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetSecondClickFiresCallback)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		parent_click_count = 0;
		Event e;
		e.event.mouse_event.x = 10;
		e.event.mouse_event.y = 10;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event_type = EventType::MouseUp;
		
		Widget* w = new Widget();

		boost::signals::scoped_connection c = w->OnFocusedClick.connect(parent_callback);
		w->HandleEvent(e);
		CHECK_EQUAL(0, parent_click_count);
		w->HandleEvent(e);
		CHECK_EQUAL(1, parent_click_count);
		w->HandleEvent(e);
		CHECK_EQUAL(2, parent_click_count);
		Widget::ClearRoot();
	}
}


TEST_FIXTURE(SDL_fixture, HierachalMouseClickHandling)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		child_click_count = 0;
		parent_click_count = 0;

		Event e;
		e.event.mouse_event.x = 50;
		e.event.mouse_event.y = 10;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event_type = EventType::MouseUp;
		
		Widget* parent = new Widget();
		Widget* child = new Widget();
		child->SetSize(Vector2i(20,20));
		child->SetPosition(Vector2i(40, 5));
		parent->AddChild(child);

		boost::signals::scoped_connection c = parent->OnClick.connect(parent_callback);
		boost::signals::scoped_connection c2 = child->OnClick.connect(child_callback);

		parent->HandleEvent(e);
		CHECK_EQUAL(1, child_click_count);
		CHECK_EQUAL(0, parent_click_count);

		e.event.mouse_event.x = 0;
		e.event.mouse_event.y = 0;
		parent->HandleEvent(e);
		CHECK_EQUAL(1, child_click_count);
		CHECK_EQUAL(1, parent_click_count);
		Widget::ClearRoot();
	}
}
