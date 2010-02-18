#include "Logger.h"
#include "ItemBrowserWidget.h"

ItemBrowserWidget::ItemBrowserWidget(vector<std::string> _items, Vector2i _grid_size, Vector2i _item_size)
: Widget()
{
	items_ = _items;
	grid_size_ = _grid_size;
	item_size_ = _item_size;
	page_ = 0;
	size_ = _grid_size * _item_size;
	//SetRejectsFocus(true);
	SetHidesHighlight(true);
	delete blit_rect_;
	delete back_rect_;
	blit_rect_ = new BlittableRect(size_);
	back_rect_ = new BlittableRect(size_);
	back_rect_->Fill(255, 255, 255, 255); //Clear the image

	BlittableRect* left = new BlittableRect("BrowseLeft.png");
	BlittableRect* scaled_left = left->Resize(_item_size);
	delete left;
	BlittableRect* right = new BlittableRect("BrowseRight.png");
	BlittableRect* scaled_right = right->Resize(_item_size);
	delete right;

	left_move_ = new Widget(scaled_left);
	left_move_->OnClick.connect(boost::bind(&ItemBrowserWidget::PrevPage, this, _1));
	left_move_->SetPosition(Vector2i(0, size_.y - item_size_.y));
	AddChild(left_move_);
	right_move_ = new Widget(scaled_right);
	right_move_->OnClick.connect(boost::bind(&ItemBrowserWidget::NextPage, this, _1));
	right_move_->SetPosition(Vector2i(size_.x - item_size_.x, size_.y - item_size_.y));
	AddChild(right_move_);

	PerformItemLayout();
	last_clicked_ = NULL;
	allow_scroll_ = true;
}

ItemBrowserWidget::~ItemBrowserWidget(void)
{
}

void ItemBrowserWidget::PerformItemLayout()
{
	for(vector<Widget*>::iterator it = clickable_items_.begin(); it != clickable_items_.end(); it++)
	{
		RemoveChild(*it);
		delete *it;
	}
	clickable_items_.clear();

	int max_number_items = grid_size_.x * grid_size_.y;
	if(allow_scroll_)
		max_number_items -= 2;
	int number_items = static_cast<int>(items_.size()) - page_ * max_number_items;
	if(number_items > max_number_items)
		number_items = max_number_items;
	for(int item = 0; item < number_items; item++)
	{
		int x = (item % grid_size_.x) * item_size_.x;
		
		if(allow_scroll_ && ((item / grid_size_.x) == grid_size_.y - 1))
			x += item_size_.x;
		int y = (item / grid_size_.x) * item_size_.y;

		BlittableRect* blittable = NULL;
		OnItemRender(this, &blittable, items_[item + page_ * max_number_items]);
		BlittableRect* scaled = NULL;
		if(blittable)
		{ //Downsample to correct size
			scaled = blittable->Resize(item_size_);
			delete blittable;
		} else
		{
			blittable = new BlittableRect("ErrorLoading.png");
			scaled = blittable->Resize(item_size_);
			delete blittable;
		}
		Widget* item_widget = new Widget(scaled);
		item_widget->SetAllowDrag(true);

		item_widget->SetPosition(Vector2i(x, y));
		item_widget->OnClick.connect(boost::bind(&ItemBrowserWidget::ItemClick, this, _1));
		item_widget->SetTag(items_[item + page_ * max_number_items]);
		item_widget->OnFocusedClick.connect(boost::bind(&ItemBrowserWidget::ItemFocusedClick, this, _1));
		item_widget->OnDragStart.connect(boost::bind(&ItemBrowserWidget::ItemDragStart, this, _1, _2));
		item_widget->OnDragEnter.connect(boost::bind(&ItemBrowserWidget::ItemDragEnter, this, _1, _2));
		item_widget->OnDragReset.connect(boost::bind(&ItemBrowserWidget::ItemDragReset, this, _1, _2));
		item_widget->OnDragLand.connect(boost::bind(&ItemBrowserWidget::ItemDragLand, this, _1, _2));
		AddChild(item_widget);
		clickable_items_.push_back(item_widget);
	}
	Invalidate();
}

void ItemBrowserWidget::NextPage(Widget* /*_widget*/)
{
	int old_page = page_;
	int max_number_items = grid_size_.x * grid_size_.y - 2;
	int number_items = static_cast<int>(items_.size()) - page_ * max_number_items;
	if(number_items > max_number_items)
		page_++;
	else
		page_ = 0;
	PerformItemLayout();
	if(page_ != old_page)
		OnPageChange(this, old_page, page_);
}

void ItemBrowserWidget::PrevPage(Widget* /*_widget*/)
{
	int old_page = page_;
	int max_number_items = grid_size_.x * grid_size_.y - 2;
	if(page_ > 0)
		page_--;
	else
	{
		page_ = (int)items_.size() / max_number_items;
		if(items_.size() % max_number_items == 0)
			page_--;
	}

	//Small numbers of items could result in negative page
	if(page_ < 0)
		page_ = 0;
	PerformItemLayout();
	if(page_ != old_page)
		OnPageChange(this, old_page, page_);
}

int ItemBrowserWidget::GetPageCount()
{
	int max_number_items = grid_size_.x * grid_size_.y - 2;
	if(items_.size() == 0)
		return 1;
	return ((static_cast<int>(items_.size()) - 1) / max_number_items) + 1;
}

void ItemBrowserWidget::SetItems(vector<std::string> _items)
{
	page_ = 0;
	items_ = _items;
	PerformItemLayout();
}

void ItemBrowserWidget::SetPage(int _page)
{
	int max_page = GetPageCount() - 1;
	int old_page = page_;
	page_ = _page;

	if(page_ < 0)
		page_ = 0;
	if(page_ > max_page)
		page_ = max_page;

	if(page_ != old_page)
		OnPageChange(this, old_page, page_);
}

void ItemBrowserWidget::ItemClick(Widget* _widget)
{
	int y = _widget->GetPosition().y / item_size_.y;
	int x = _widget->GetPosition().x / item_size_.x;
	int page_index = x + grid_size_.x * y;
	if(y == grid_size_.y - 1 && allow_scroll_)
		page_index--;

	int item_index = page_ * (grid_size_.x * grid_size_.y - 2) + page_index;

	OnItemClick(this, items_[item_index]);
	if(_widget != last_clicked_)
	{
		last_clicked_ = _widget;
		OnItemSelectedChanged(this, items_[item_index]);
	}
}

void ItemBrowserWidget::ItemFocusedClick(Widget* _widget)
{
	int y = _widget->GetPosition().y / item_size_.y;
	int x = _widget->GetPosition().x / item_size_.x;
	int page_index = x + grid_size_.x * y;
	if(y == grid_size_.y - 1 && allow_scroll_)
		page_index--;

	int item_index = page_ * (grid_size_.x * grid_size_.y - 2) + page_index;

	OnItemFocusedClick(this, items_[item_index]);
}

void ItemBrowserWidget::SetAllowScroll(bool _allow_scroll)
{
	if(_allow_scroll && !allow_scroll_)
	{
		BlittableRect* left = new BlittableRect("BrowseLeft.png");
		BlittableRect* scaled_left = left->Resize(item_size_);
		delete left;
		BlittableRect* right = new BlittableRect("BrowseRight.png");
		BlittableRect* scaled_right = right->Resize(item_size_);
		delete right;

		left_move_ = new Widget(scaled_left);
		left_move_->OnClick.connect(boost::bind(&ItemBrowserWidget::PrevPage, this, _1));
		left_move_->SetPosition(Vector2i(0, size_.y - item_size_.y));
		AddChild(left_move_);
		Widget* right_move_ = new Widget(scaled_right);
		right_move_->OnClick.connect(boost::bind(&ItemBrowserWidget::NextPage, this, _1));
		right_move_->SetPosition(Vector2i(size_.x - item_size_.x, size_.y - item_size_.y));
		AddChild(right_move_);
	} else if(!_allow_scroll && allow_scroll_)
	{
		if(left_move_)
			delete left_move_;
		left_move_ = NULL;
		if(right_move_)
			delete right_move_;
		right_move_ = NULL;
		//children_.erase(std::remove(children_.begin(), children_.end(), left_move_), children_.end());
		//children_.erase(std::remove(children_.begin(), children_.end(), right_move_), children_.end());
	}
	allow_scroll_ = _allow_scroll;
}

void ItemBrowserWidget::ItemDragStart(Widget* _widget, DragEventArgs* _args)
{
	OnItemDragStart(_widget, _args);
}

void ItemBrowserWidget::ItemDragEnter(Widget* _widget, DragEventArgs* _args)
{
	OnItemDragEnter(_widget, _args);
}

void ItemBrowserWidget::ItemDragReset(Widget* _widget, DragEventArgs* _args)
{
	OnItemDragReset(_widget, _args);
}

void ItemBrowserWidget::ItemDragLand(Widget* _widget, DragEventArgs* _args)
{
	OnItemDragLand(_widget, _args);
}