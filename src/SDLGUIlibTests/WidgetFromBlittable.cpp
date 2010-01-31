#include "stdafx.h"
#include "Widget.h"

TEST_FIXTURE(SDL_fixture, CreateWidgetFromBlittable)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		BlittableRect* br = new BlittableRect(Vector2i(120, 120));
		Widget* p_widget = new Widget(br);
		CHECK_EQUAL(Vector2i(120, 120), p_widget->GetSize());
		delete p_widget;
	}
}

TEST_FIXTURE(SDL_fixture, BlittableScaling)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		BlittableRect br = BlittableRect(Vector2i(100, 100));
		BlittableRect* br2 = br.Resize(Vector2i(50, 50));
		CHECK_EQUAL(Vector2i(50, 50), br2->GetSize());
		delete br2;
	}
}

