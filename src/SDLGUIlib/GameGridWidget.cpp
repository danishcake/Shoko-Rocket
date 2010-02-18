#include "Logger.h"
#include "GameGridWidget.h"

GameGridWidget::GameGridWidget(Vector2i _grid_size, Vector2i _item_size)
{
	grid_size_ = _grid_size;
	item_size_ = _item_size;
	rejects_focus_ = true;
	size_ = _grid_size * _item_size;
	delete blit_rect_;
	delete back_rect_;
	blit_rect_ = new BlittableRect(size_);
	back_rect_ = new BlittableRect(size_);
	offset_ = Vector2i(0, 0);

	back_rect_->Fill(0,0,0,0); //Clear the image and make transparent

	OnKeyUp.connect(boost::bind(&GameGridWidget::GridKeyUp, this, _1, _2));
	OnMouseClick.connect(boost::bind(&GameGridWidget::GridMouseClick, this, _1, _2));			
	SetAllowDrag(true);
	OnDragReset.connect(boost::bind(&GameGridWidget::GridDragReset, this, _1, _2));
	OnDragStart.connect(boost::bind(&GameGridWidget::GridDragStart, this, _1, _2));
}

GameGridWidget::~GameGridWidget(void)
{
}

void GameGridWidget::GridMouseClick(Widget* /*_widget*/, MouseEventArgs args)
{
	MouseEventArgs e;
	e.x = (args.x + offset_.x) / item_size_.x;
	e.y = (args.y + offset_.y) / item_size_.y;
	e.btns = args.btns;
	OnGridClick(this, e);
}

void GameGridWidget::GridKeyUp(Widget* /*_widget*/, KeyPressEventArgs args)
{
	GridKeyPressEventArgs e;
	e.x = (last_mouse_position_.x + offset_.x) / item_size_.x;
	e.y = (last_mouse_position_.y + offset_.y) / item_size_.y;
	e.key_code = args.key_code;
	OnGridKeyUp(this, e);
}

void GameGridWidget::GridDragStart(Widget* /*_widget*/, DragEventArgs* _args)
{
	_args->drag_type = 1;
}

void GameGridWidget::GridDragReset(Widget* /*_widget*/, DragEventArgs* _args)
{
	GridGestureEventArgs e;
	e.x = (_args->sx + offset_.x) / item_size_.x;
	e.y = (_args->sy + offset_.y) / item_size_.y;
	e.direction = GestureDirection::Center;

	float angle = atan2f(static_cast<float>(_args->x), -static_cast<float>(_args->y));
	float length = Vector2f(static_cast<float>(_args->x), -static_cast<float>(_args->y)).length();

	if(length > 5)
	{
		if(angle > -M_PI / 4.0 && angle < M_PI / 4.0)
		{
			e.direction = GestureDirection::North;
		} else if(angle > (-M_PI / 4.0) + (M_PI / 2.0) && angle < (M_PI / 4.0) + (M_PI / 2.0))
		{
			e.direction = GestureDirection::East;
		} else if(angle > (-M_PI / 4.0) - (M_PI / 2.0) && angle < (M_PI / 4.0) - (M_PI / 2.0))
		{
			e.direction = GestureDirection::West;
		} else if(angle > (-M_PI / 4.0) + (M_PI) || angle < (M_PI / 4.0) + (M_PI))
		{
			e.direction = GestureDirection::South;
		}
	}

	OnGridGesture(this, e);
}