#pragma once
#include "vmath.h"

class Widget;

namespace EventType
{
	enum Enum
	{
		MouseMove, MouseDown, MouseUp, KeyLeft, KeyRight, KeyUp, KeyDown, KeyEnter, OtherKeypress
	};
}

namespace MouseButton
{
	enum Enum 
	{
		None = 0, Left = 1, Right=2, Middle=4, ScrollUp = 8, ScrollDown = 16
	};
}

struct MouseEventArgs
{
	int x;
	int y;
	MouseButton::Enum btns;
};

struct KeyPressEventArgs
{
	int key_code;
};

struct DragEventArgs
{
	int x;
	int y;
	int drag_type;
	bool drag_accepted;
	void* data;

	DragEventArgs()
	{
		x = 0;
		y = 0;
		drag_type = 0;
		data = NULL;
		drag_accepted = false;
	}
};

union EventUnion
{
	MouseEventArgs mouse_event;
	KeyPressEventArgs key_event;
};

struct Event
{
	EventType::Enum event_type;
	EventUnion event;
	
	Event()
	{
		event_type = EventType::MouseUp;
		event.mouse_event.btns = MouseButton::Left;
		event.mouse_event.x = 0;
		event.mouse_event.y = 0;
	}
	Event(EventType::Enum _event_type)
	{
		event_type = _event_type;
	}
};