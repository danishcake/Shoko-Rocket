#include "stdafx.h"
#include <MenuWidget.h>

namespace
{
	int start_count = 0;
	void start_callback(Widget* _widget)
	{
		start_count++;
	}

	int options_count = 0;
	void options_callback(Widget* _widget)
	{
		options_count++;
	}

	int exit_count = 0;
	void exit_callback(Widget* _widget)
	{
		exit_count++;
	}
}

TEST_FIXTURE(SDL_fixture, MenuAutoLinks)
{
	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		MenuWidget* menu = new MenuWidget();
		Widget* pStart =	menu->AddMenuItem("Start");
		Widget* pOptions =	menu->AddMenuItem("Options");
		Widget* pExit =		menu->AddMenuItem("Exit");

		pStart->OnClick.connect(start_callback);
		pOptions->OnClick.connect(options_callback);
		pExit->OnClick.connect(exit_callback);
		
		menu->SetFocusToFirst();
		CHECK_EQUAL(menu->GetChildren().at(0), Widget::GetWidgetWithFocus());
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyDown));
		CHECK_EQUAL(menu->GetChildren().at(1), Widget::GetWidgetWithFocus());
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyDown));
		CHECK_EQUAL(menu->GetChildren().at(2), Widget::GetWidgetWithFocus());
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyDown));
		CHECK_EQUAL(menu->GetChildren().at(0), Widget::GetWidgetWithFocus());

		CHECK_EQUAL(Vector2i(0, 0), menu->GetChildren().at(0)->GetPosition());
		CHECK_EQUAL(Vector2i(8, 40), menu->GetChildren().at(1)->GetPosition());
		CHECK_EQUAL(Vector2i(16, 80), menu->GetChildren().at(2)->GetPosition());

		CHECK_EQUAL(0, start_count);
		CHECK_EQUAL(0, options_count);
		CHECK_EQUAL(0, exit_count);
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyEnter));
		CHECK_EQUAL(menu->GetChildren().at(0), Widget::GetWidgetWithFocus());
		CHECK_EQUAL(1, start_count);
		CHECK_EQUAL(0, options_count);
		CHECK_EQUAL(0, exit_count);

		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyUp));
		CHECK_EQUAL(menu->GetChildren().at(2), Widget::GetWidgetWithFocus());
		Widget::GetWidgetWithFocus()->HandleEvent(Event(EventType::KeyEnter));
		CHECK_EQUAL(1, start_count);
		CHECK_EQUAL(0, options_count);
		CHECK_EQUAL(1, exit_count);
	}
}

