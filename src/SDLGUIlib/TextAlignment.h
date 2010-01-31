#pragma once
#include <string>

namespace TextAlignment
{
	enum Enum
	{
		TopLeft, Top, TopRight, Left, Centre, Right, BottomLeft, Bottom, BottomRight
	};
}

struct WidgetText
{
public:
	TextAlignment::Enum alignment;
	std::string text;
	WidgetText(){text=""; alignment=TextAlignment::Centre;}
};