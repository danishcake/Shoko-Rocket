#include "stdafx.h"
#include "GameGridWidget.h"

namespace
{
	int click_count = 0;
	Vector2i last_click_coord;
	void grid_callback(Widget* /*_widget*/, MouseEventArgs _event)
	{
		click_count ++;
		last_click_coord.x = _event.x;
		last_click_coord.y = _event.y;
	}

	int last_keycode = 0;
	void wasd_callback(Widget* /*_widget*/, KeyPressEventArgs _event)
	{
		last_keycode = _event.key_code;
	}
}

TEST_FIXTURE(SDL_fixture, GameGridBasics)
{
	UNITTEST_TIME_CONSTRAINT(500);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		GameGridWidget* p_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32,32));
		CHECK(p_widget);
		CHECK_EQUAL(Vector2i(12, 9), p_widget->GetGridSize());
		CHECK_EQUAL(Vector2i(32, 32), p_widget->GetItemSize());
		CHECK(p_widget->GetRejectsFocus());
		CHECK_EQUAL(9 * 12, p_widget->GetChildren().size());

		delete p_widget;
	}
}

TEST_FIXTURE(SDL_fixture, GameGridEvents)
{
	UNITTEST_TIME_CONSTRAINT(500);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		GameGridWidget* p_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32,32));
		CHECK(p_widget);
		
		p_widget->OnGridClick.connect(grid_callback);

		Event e;
		e.event.mouse_event.x = 5 * 32 + 10;
		e.event.mouse_event.y = 5 * 32 + 10;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event_type = EventType::MouseUp;
		p_widget->HandleEvent(e);

		CHECK_EQUAL(Vector2i(5, 5), last_click_coord);
		CHECK_EQUAL(1, click_count);

		delete p_widget;
	}
}

TEST_FIXTURE(SDL_fixture, KeypressEvents)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		GameGridWidget* p_widget = NULL;
		{
			UNITTEST_TIME_CONSTRAINT(500);
			p_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32,32));
		}
	
		Event e;
		e.event_type = EventType::OtherKeypress;
		e.event.key_event.key_code = SDLK_a;
		p_widget->OnKeyUp.connect(wasd_callback);
		p_widget->HandleEvent(e);

		CHECK_EQUAL(SDLK_a, last_keycode);
		delete p_widget;
	}
}
