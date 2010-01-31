#include "stdafx.h"
#include <Widget.h>


TEST_FIXTURE(SDL_fixture, FadeTests)
{
	Widget::SetFade(0.5f);
	CHECK_EQUAL(0.5f, Widget::GetFade());
}