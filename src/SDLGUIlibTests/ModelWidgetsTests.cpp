#include "stdafx.h"
#include "Widget.h"

using std::vector;
using std::string;

TEST_FIXTURE(SDL_fixture, ModalDefaults)
{
	UNITTEST_TIME_CONSTRAINT(50);
 	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		CHECK_EQUAL((Widget*)NULL, Widget::GetModalWidget());
		Widget* p_widget = new Widget();
		CHECK_EQUAL((Widget*)NULL, Widget::GetModalWidget());
		CHECK_EQUAL(false, p_widget->HasModal());
	}
}

TEST_FIXTURE(SDL_fixture, ModelWidgetSelection)
{
	UNITTEST_TIME_CONSTRAINT(50);
 	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* p_widgetA = new Widget();
		Widget* p_widgetB = new Widget();

		Widget::SetModalWidget(p_widgetA);
		CHECK_EQUAL(p_widgetA, Widget::GetModalWidget());
		CHECK_EQUAL(true, p_widgetA->HasModal());
		CHECK_EQUAL(false, p_widgetB->HasModal());

		Widget::SetModalWidget(p_widgetB);
		CHECK_EQUAL(p_widgetB, Widget::GetModalWidget());
		CHECK_EQUAL(false, p_widgetA->HasModal());
		CHECK_EQUAL(true, p_widgetB->HasModal());

		p_widgetA->SetModal(true);
		CHECK_EQUAL(p_widgetA, Widget::GetModalWidget());
		CHECK_EQUAL(true, p_widgetA->HasModal());
		CHECK_EQUAL(false, p_widgetB->HasModal());

		p_widgetA->SetModal(false);
		CHECK_EQUAL((Widget*)NULL, Widget::GetModalWidget());
		CHECK_EQUAL(false, p_widgetA->HasModal());
		CHECK_EQUAL(false, p_widgetB->HasModal());
	}
}

namespace
{
	int click_a_count = 0;
	void click_a(Widget* _widget)
	{
		click_a_count++;
	}

	int click_b_count = 0;
	void click_b(Widget* _widget)
	{
		click_b_count++;
	}
}

TEST_FIXTURE(SDL_fixture, ModelEvents)
{
	UNITTEST_TIME_CONSTRAINT(50);
 	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		click_a_count = 0;
		click_b_count = 0;
		CHECK_EQUAL((Widget*)NULL, Widget::GetModalWidget());
		Widget* p_widgetA = new Widget();
		p_widgetA->OnClick.connect(click_a);
		Widget* p_widgetB = new Widget();
		p_widgetB->SetPosition(Vector2i(p_widgetA->GetSize().x, 0));
		p_widgetB->OnClick.connect(click_b);

		Event e;
		e.event_type = EventType::MouseUp;
		e.event.mouse_event.btns == MouseButton::Left;
		e.event.mouse_event.x = p_widgetA->GetSize().x / 2;
		e.event.mouse_event.y = p_widgetA->GetSize().y / 2;
		CHECK_EQUAL(0, click_a_count);
		p_widgetA->HandleEvent(e);
		CHECK_EQUAL(1, click_a_count);
		Widget::SetModalWidget(p_widgetA);
		p_widgetA->HandleEvent(e);
		CHECK_EQUAL(2, click_a_count);

		Widget::SetModalWidget(p_widgetB);
		p_widgetA->HandleEvent(e);
		CHECK_EQUAL(2, click_a_count);

		p_widgetB->HandleEvent(e);
		CHECK_EQUAL(1, click_b_count);
	}
}

TEST_FIXTURE(SDL_fixture, ModalHierarchy)
{
	UNITTEST_TIME_CONSTRAINT(50);
 	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		Widget* widget_a = new Widget();
		Widget* widget_a2 = new Widget();
		Widget* widget_b = new Widget();
		widget_a->AddChild(widget_a2);

		widget_a->SetModal(true);

		CHECK_EQUAL(true, widget_a->HasModal());
		CHECK_EQUAL(true, widget_a->HasOrInheritsModal());
		
		CHECK_EQUAL(false, widget_a2->HasModal());
		CHECK_EQUAL(true, widget_a2->HasOrInheritsModal());

		CHECK_EQUAL(false, widget_b->HasOrInheritsModal());
		CHECK_EQUAL(false, widget_b->HasModal());
	}	
}