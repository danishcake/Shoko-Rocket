#include "stdafx.h"
#include <Widget.h>
#include <Event.h>

namespace
{
	int widgetevent_callback_count = 0;
	void widgetevent_callback(Widget* _widget)
	{
		widgetevent_callback_count++;
	}
}

TEST_FIXTURE(SDL_fixture, FlatWidgets)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Event e;
		e.event_type = EventType::KeyLeft;
		Widget* left = new Widget();
		Widget* right = new Widget();
		Widget* left_bottom = new Widget();
		Widget* right_bottom = new Widget();
		left->LinkRight(right);
		left->LinkDown(left_bottom);
		right->LinkLeft(left);
		right->LinkDown(right_bottom);
		left_bottom->LinkUp(left);
		left_bottom->LinkRight(right_bottom);
		right_bottom->LinkUp(right);
		right_bottom->LinkLeft(left_bottom);


		boost::signals::scoped_connection c = left->OnGainFocus.connect(widgetevent_callback);
		left->SetFocus();
		CHECK_EQUAL(1, widgetevent_callback_count);

		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(true, left->HasFocus());
		CHECK_EQUAL(false, right->HasFocus());

		e.event_type = EventType::KeyRight;
		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(false, left->HasFocus());
		CHECK_EQUAL(true, right->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(false, left->HasFocus());
		CHECK_EQUAL(true, right->HasFocus());
		
		e.event_type = EventType::KeyDown;
		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(true, right_bottom->HasFocus());

		e.event_type = EventType::KeyLeft;
		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(true, left_bottom->HasFocus());

		e.event_type = EventType::KeyUp;
		Widget::GetWidgetWithFocus()->HandleEvent(e);
		CHECK_EQUAL(true, left->HasFocus());
		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, HierarchalWidgets)
{
	/* Layout 
	|------------------------------|  |--------|
	|A                             |  |E       |
	| |-----------|   |-------|    |  |        |
	| |B          |   |C      |    |  |        |
	| |           |   |-------|    |  |        |
	| |           |                |  |        |
	| |           |                |  |        |
	| |           |   |-------|    |  |        |
	| |           |   |D      |    |  |        |
	| |-----------|   |-------|    |  |        |
	|                              |  |        |
	|------------------------------|  |--------|
	                                            
	|------------------------------------------|
	|F                                         |
	|------------------------------------------|
	*/
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* A = new Widget();
		Widget* B = new Widget();
		Widget* C = new Widget();
		Widget* D = new Widget();
		Widget* E = new Widget();
		Widget* F = new Widget();

		A->AddChild(B);
		A->AddChild(C);
		A->AddChild(D);

		A->LinkInnerRight(B);
		A->LinkInnerLeft(C);
		A->LinkInnerDown(B);
		A->LinkInnerUp(B);
		A->LinkRight(E);
		E->LinkLeft(A);

		B->LinkRight(C);
		C->LinkLeft(B);
		D->LinkLeft(B);
		
		A->LinkDown(F);
		E->LinkDown(F);
		F->LinkUp(A);

		A->SetFocus();
		CHECK_EQUAL(true, A->HasFocus());
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyDown));
		CHECK_EQUAL(true, B->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyDown));
		CHECK_EQUAL(true, F->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyUp));
		CHECK_EQUAL(true, A->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyUp));
		CHECK_EQUAL(true, B->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyUp));
		CHECK_EQUAL(true, A->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyRight));
		CHECK_EQUAL(true, B->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyRight));
		CHECK_EQUAL(true, C->HasFocus());

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyRight));
		CHECK_EQUAL(true, E->HasFocus());
		Widget::ClearRoot();
	}
}

