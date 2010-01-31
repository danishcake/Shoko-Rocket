#include "stdafx.h"
#include <Widget.h>
#include <Event.h>

namespace
{
	int drag_start_count = 0;
	void WidgetDragStart(Widget* _widget, DragEventArgs* _args)
	{
		drag_start_count++;
		_args->drag_type = 1; //Important - ignored if zero
	}

	int drag_reset_count = 0;
	Vector2i drag_reset_position;
	void WidgetDragReset(Widget* _widget, DragEventArgs* _args)
	{
		drag_reset_count++;
		drag_reset_position = Vector2i(_args->x, _args->y);
	}

	void WidgetDragEnterRejected(Widget* _widget, DragEventArgs* _args)
	{
		_args->drag_accepted = false;
	}

	void WidgetDragEnterAccepted(Widget* _widget, DragEventArgs* _args)
	{
		_args->drag_accepted = true;
	}

	int sucessful_landings = 0;
	void WidgetDragLand(Widget* _widget, DragEventArgs* _args)
	{
		if(_args->drag_type == 1)
			sucessful_landings++;
	}
}

TEST_FIXTURE(SDL_fixture, WidgetDragSources)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* widget = new Widget();
		CHECK_EQUAL(false, widget->GetAllowDrag());
		widget->SetAllowDrag(true);
		CHECK_EQUAL(true, widget->GetAllowDrag());


		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetStartDragEvent)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		drag_start_count = 0;
		Widget* widget = new Widget();
		widget->SetAllowDrag(true);
		widget->OnDragStart.connect(WidgetDragStart);
	
		SDL_Event e;
		e.type = SDL_MOUSEMOTION;
		e.motion.x = 10;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEBUTTONDOWN;
		e.button.x = 10;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(0, drag_start_count);

		e.type = SDL_MOUSEMOTION;
		e.motion.x = 11;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(1, drag_start_count);
		CHECK_EQUAL(widget, Widget::GetDraggedWidget());

		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetDragResetEventOverSelf)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		drag_reset_count = 0;
		drag_start_count = 0;
		Widget* widget = new Widget();
		widget->SetAllowDrag(true);
		widget->OnDragStart.connect(WidgetDragStart);
		widget->OnDragReset.connect(WidgetDragReset);

	
		SDL_Event e;
		e.type = SDL_MOUSEMOTION;
		e.motion.x = 10;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEBUTTONDOWN;
		e.button.x = 10;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEMOTION;
		e.motion.x = 11;
		Widget::DistributeSDLEvents(&e);

		//Mouse drop on top of dragged widget results in a reset
		e.type = SDL_MOUSEBUTTONUP;
		e.button.x = 12;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(1, drag_reset_count); 
		CHECK_EQUAL((Widget*)NULL, Widget::GetDraggedWidget());
		CHECK_EQUAL(Vector2i(1, 0), drag_reset_position);

		Widget::ClearRoot();
	}
}

TEST_FIXTURE(SDL_fixture, WidgetDragResetEventOverEmpty)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		drag_reset_count = 0;
		drag_start_count = 0;
		Widget* widget = new Widget();
		widget->SetAllowDrag(true);
		widget->OnDragStart.connect(WidgetDragStart);
		widget->OnDragReset.connect(WidgetDragReset);

	
		SDL_Event e;
		e.type = SDL_MOUSEMOTION;
		e.motion.x = 10;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEBUTTONDOWN;
		e.button.x = 10;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEMOTION;
		e.motion.x = 11;
		Widget::DistributeSDLEvents(&e);

		//Mouse drop on top of dragged widget results in a reset
		e.type = SDL_MOUSEBUTTONUP;
		e.button.x = 16;
		e.button.y = 15;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(1, drag_reset_count);
		CHECK_EQUAL((Widget*)NULL, Widget::GetDraggedWidget());
		CHECK_EQUAL(Vector2i(5, 5), drag_reset_position);

		Widget::ClearRoot();
	}
} 

TEST_FIXTURE(SDL_fixture, WidgetDragResetEventOverRejectingWidget)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		drag_reset_count = 0;
		drag_reset_position = Vector2i(0, 0);
		drag_start_count = 0;
		Widget* widget = new Widget();
		widget->SetSize(Vector2i(50, 50));
		widget->SetAllowDrag(true);
		widget->OnDragStart.connect(WidgetDragStart);
		widget->OnDragReset.connect(WidgetDragReset);
		

		Widget* widget2 = new Widget();
		widget2->SetSize(Vector2i(50, 50));
		widget2->SetPosition(Vector2i(50, 0));
		widget2->OnDragEnter.connect(WidgetDragEnterRejected);

		
		SDL_Event e;
		e.type = SDL_MOUSEMOTION;
		e.motion.x = 10;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEBUTTONDOWN;
		e.button.x = 10;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEMOTION;
		e.motion.x = 55;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		//Mouse drop on top of dragged widget results in a reset
		e.type = SDL_MOUSEBUTTONUP;
		e.button.x = 65;
		e.button.y = 15;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(1, drag_reset_count);
		CHECK_EQUAL((Widget*)NULL, Widget::GetDraggedWidget());
		CHECK_EQUAL(Vector2i(10, 5), drag_reset_position);

		Widget::ClearRoot();
	}
} 

TEST_FIXTURE(SDL_fixture, WidgetDragResetEventOverAccceptingWidget)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		drag_reset_count = 0;
		drag_reset_position = Vector2i(0, 0);
		drag_start_count = 0;
		sucessful_landings = 0;
		Widget* widget = new Widget();
		widget->SetSize(Vector2i(50, 50));
		widget->SetAllowDrag(true);
		widget->OnDragStart.connect(WidgetDragStart);
		widget->OnDragReset.connect(WidgetDragReset);
		

		Widget* widget2 = new Widget();
		widget2->SetSize(Vector2i(50, 50));
		widget2->SetPosition(Vector2i(50, 0));
		widget2->OnDragEnter.connect(WidgetDragEnterAccepted);
		widget2->OnDragLand.connect(WidgetDragLand);

		
		SDL_Event e;
		e.type = SDL_MOUSEMOTION;
		e.motion.x = 10;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEBUTTONDOWN;
		e.button.x = 10;
		e.button.y = 10;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);

		e.type = SDL_MOUSEMOTION;
		e.motion.x = 55;
		e.motion.y = 10;
		Widget::DistributeSDLEvents(&e);

		//Mouse drop on top of dragged widget results in a reset
		e.type = SDL_MOUSEBUTTONUP;
		e.button.x = 65;
		e.button.y = 15;
		e.button.button = SDL_BUTTON_LEFT;
		Widget::DistributeSDLEvents(&e);
		CHECK_EQUAL(0, drag_reset_count);
		CHECK_EQUAL((Widget*)NULL, Widget::GetDraggedWidget());
		CHECK_EQUAL(1, sucessful_landings);

		//TODO accept count & position?
		Widget::ClearRoot();
	}
} 