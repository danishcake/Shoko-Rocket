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

	back_rect_->Fill(0,0,0,0); //Clear the image and make transparent

	for(int x = 0; x < grid_size_.x; x++)
	{
		for(int y = 0; y < grid_size_.y; y++)
		{
			Widget* item = new Widget();
			item->SetSize(item_size_);
			item->SetAllowDrag(true);
			item->SetPosition(Vector2i(item_size_.x * x, item_size_.y * y));

			item->OnKeyUp.connect(boost::bind(&GameGridWidget::GridKeyUp, this, _1, _2));
			item->OnMouseClick.connect(boost::bind(&GameGridWidget::GridMouseClick, this, _1, _2));			
			item->OnDragReset.connect(boost::bind(&GameGridWidget::GridDragReset, this, _1, _2));
			item->OnDragStart.connect(boost::bind(&GameGridWidget::GridDragStart, this, _1, _2));
			AddChild(item);
		}
	}
}

GameGridWidget::~GameGridWidget(void)
{
}

void GameGridWidget::GridMouseClick(Widget* _widget, MouseEventArgs args)
{
	MouseEventArgs e;
	e.x = _widget->GetPosition().x / _widget->GetSize().x;
	e.y = _widget->GetPosition().y / _widget->GetSize().y;
	e.btns = args.btns;
	OnGridClick(this, e);
}

void GameGridWidget::GridKeyUp(Widget* _widget, KeyPressEventArgs args)
{
	GridKeyPressEventArgs e;
	e.x = _widget->GetPosition().x / _widget->GetSize().x;
	e.y = _widget->GetPosition().y / _widget->GetSize().y;
	e.key_code = args.key_code;
	OnGridKeyUp(this, e);
}

void GameGridWidget::GridDragStart(Widget* _widget, DragEventArgs* _args)
{
	_args->drag_type = 1;
}

void GameGridWidget::GridDragReset(Widget* _widget, DragEventArgs* _args)
{
	GridGestureEventArgs e;
	e.x = _widget->GetPosition().x / _widget->GetSize().x;
	e.y = _widget->GetPosition().y / _widget->GetSize().y;
	e.direction = GestureDirection::Center;

	float angle = atan2f(_args->x, -_args->y);
	float length = Vector2f(_args->x, -_args->y).length();

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