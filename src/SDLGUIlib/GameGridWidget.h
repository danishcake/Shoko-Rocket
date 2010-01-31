#pragma once
#include "Widget.h"

struct GridKeyPressEventArgs
{
	int x;
	int y;
	int key_code;
};

namespace GestureDirection
{
	enum Enum
	{
		Center, North, West, South, East
	};
}

struct GridGestureEventArgs
{
	int x;
	int y;
	GestureDirection::Enum direction;
};

class GameGridWidget :
	public Widget
{
protected:
	Vector2i grid_size_;
	Vector2i item_size_;

	void GridMouseClick(Widget* _widget, MouseEventArgs _args);
	void GridKeyUp(Widget* _widget, KeyPressEventArgs _args);
	void GridDragReset(Widget* _widget, DragEventArgs* _args);
	void GridDragStart(Widget* _widget, DragEventArgs* _args);
public:
	/* Constructors */
	GameGridWidget(Vector2i _grid_size, Vector2i _item_size);
	virtual ~GameGridWidget(void);

	/* Typedefs */
	typedef boost::signal<void (Widget*, GridKeyPressEventArgs)> GridKeyEvent;
	typedef boost::signal<void (Widget*, GridGestureEventArgs)> GridGestureEvent;

	/* Properties */
	Vector2i GetGridSize(){return grid_size_;}
	Vector2i GetItemSize(){return item_size_;}

	/* Events */
	MouseEvent OnGridClick;
	GridKeyEvent OnGridKeyUp;
	GridGestureEvent OnGridGesture;
	
};
