#include "Logger.h"
#include "Widget.h"
#include <algorithm>
#include "vmath-collisions.h"

Widget* Widget::widget_with_focus_ = NULL;
Widget* Widget::widget_with_highlight_ = NULL;
Widget* Widget::widget_with_depression_ = NULL;
Widget* Widget::widget_with_drag_ = NULL;
Widget* Widget::widget_with_modal_ = NULL;
Widget* Widget::widget_with_edit_ = NULL;
bool Widget::event_lock_ = false;
Widget::KeyEvent Widget::OnGlobalKeyUp;

DragEventArgs Widget::drag_event_args_ = DragEventArgs();
Vector2i Widget::drag_start_position_ = Vector2i(0, 0);

vector<Widget*> Widget::root_ = vector<Widget*>();
vector<Widget*> Widget::all_ = vector<Widget*>();
vector<Widget*> Widget::pending_root_ = vector<Widget*>();
vector<Widget*> Widget::pending_all_ = vector<Widget*>();

float Widget::screen_fade_ = 0.0f;
BlittableRect* Widget::screen_fade_rect_ = NULL;
BlittableRect* Widget::edit_cursor_rect_ = NULL;
double Widget::sum_time_ = 0;

template <class list_t> 
class WidgetZSort: public std::binary_function<list_t, list_t, bool> 
{
public:
	bool operator()(list_t a, list_t b) const
	{
		return a->GetZOrder() < b->GetZOrder();
	}
};


Widget::Widget(void)
{
	position_ = Vector2i(0, 0);
	size_ = Vector2i(128, 48);
	blit_rect_ = new BlittableRect(size_);
	back_rect_ = new BlittableRect(size_);
	left_link_ = NULL;
	right_link_ = NULL;
	up_link_ = NULL;
	down_link_ = NULL;
	left_inner_link_ = NULL;
	right_inner_link_ = NULL;
	up_inner_link_ = NULL;
	down_inner_link_ = NULL;
	parent_ = NULL;
	invalidated_ = true;
	rejects_focus_ = false;
	hides_highlight_ = false;
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	z_order_ = 0;
	deletion_due_ = false;
	if(event_lock_)
	{
		pending_root_.push_back(this);
		pending_all_.push_back(this);
	} else
	{
		root_.push_back(this);
		all_.push_back(this);
	}
}

Widget::Widget(std::string _filename)
{
	position_ = Vector2i(0, 0);
	back_rect_ = new BlittableRect(_filename);
	blit_rect_ = new BlittableRect(back_rect_->GetSize());
	
	size_ = blit_rect_->GetSize();
	left_link_ = NULL;
	right_link_ = NULL;
	up_link_ = NULL;
	down_link_ = NULL;
	left_inner_link_ = NULL;
	right_inner_link_ = NULL;
	up_inner_link_ = NULL;
	down_inner_link_ = NULL;
	parent_ = NULL;
	invalidated_ = true;
	rejects_focus_ = false;
	hides_highlight_ = false;
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	z_order_ = 0;
	deletion_due_ = false;
	if(event_lock_)
	{
		pending_root_.push_back(this);
		pending_all_.push_back(this);
	} else
	{
		root_.push_back(this);
		all_.push_back(this);
	}
}

Widget::Widget(BlittableRect* _blittable)
{	
	position_ = Vector2i(0, 0);
	back_rect_ = _blittable;
	blit_rect_ = new BlittableRect(back_rect_->GetSize());
	
	size_ = blit_rect_->GetSize();
	left_link_ = NULL;
	right_link_ = NULL;
	up_link_ = NULL;
	down_link_ = NULL;
	left_inner_link_ = NULL;
	right_inner_link_ = NULL;
	up_inner_link_ = NULL;
	down_inner_link_ = NULL;
	parent_ = NULL;
	invalidated_ = true;
	rejects_focus_ = false;
	hides_highlight_ = false;
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	z_order_ = 0;
	deletion_due_ = false;
	if(event_lock_)
	{
		pending_root_.push_back(this);
		pending_all_.push_back(this);
	} else
	{
		root_.push_back(this);
		all_.push_back(this);
	}
}

Widget::Widget(VerticalTile _tiles, int _height)
{
	//Width is taken from top tile
	BlittableRect toptile(_tiles.top);
	BlittableRect middletile(_tiles.middle);
	BlittableRect bottomtile(_tiles.bottom);

	position_ = Vector2i(0, 0);
	size_ = Vector2i(toptile.GetSize().x, _height);
	blit_rect_ = new BlittableRect(size_);
	back_rect_ = new BlittableRect(size_);

	back_rect_->Fill(0, 0, 0, 0);
	toptile.RawBlit(Vector2i(0,0), back_rect_);
	int center_reps = ceil(static_cast<double>(back_rect_->GetSize().y - toptile.GetSize().y - bottomtile.GetSize().y) / static_cast<double>(middletile.GetSize().y));
	for(int i = 0; i < center_reps; i++)
	{
		middletile.RawBlit(Vector2i(0, i * middletile.GetSize().y + toptile.GetSize().y), back_rect_);
	}
	bottomtile.RawBlit(Vector2i(0, back_rect_->GetSize().y - bottomtile.GetSize().y), back_rect_);

	left_link_ = NULL;
	right_link_ = NULL;
	up_link_ = NULL;
	down_link_ = NULL;
	left_inner_link_ = NULL;
	right_inner_link_ = NULL;
	up_inner_link_ = NULL;
	down_inner_link_ = NULL;
	parent_ = NULL;
	invalidated_ = true;
	rejects_focus_ = false;
	hides_highlight_ = false;
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	z_order_ = 0;
	deletion_due_ = false;
	if(event_lock_)
	{
		pending_root_.push_back(this);
		pending_all_.push_back(this);
	} else
	{
		root_.push_back(this);
		all_.push_back(this);
	}
}

Widget::Widget(HorizontalTile _tiles, int _width)
{
	//Height is taken from left tile
	BlittableRect lefttile(_tiles.left);
	BlittableRect middletile(_tiles.middle);
	BlittableRect righttile(_tiles.right);

	position_ = Vector2i(0, 0);
	size_ = Vector2i(_width, lefttile.GetSize().y);
	blit_rect_ = new BlittableRect(size_);
	back_rect_ = new BlittableRect(size_);

	back_rect_->Fill(0, 0, 0, 0);
	lefttile.RawBlit(Vector2i(0,0), back_rect_);
	int center_reps = ceil(static_cast<double>(back_rect_->GetSize().x - lefttile.GetSize().x - righttile.GetSize().x) / static_cast<double>(middletile.GetSize().x));
	for(int i = 0; i < center_reps; i++)
	{
		middletile.RawBlit(Vector2i(i * middletile.GetSize().x + lefttile.GetSize().x, 0), back_rect_);
	}
	righttile.RawBlit(Vector2i(back_rect_->GetSize().x - righttile.GetSize().x, 0), back_rect_);

	left_link_ = NULL;
	right_link_ = NULL;
	up_link_ = NULL;
	down_link_ = NULL;
	left_inner_link_ = NULL;
	right_inner_link_ = NULL;
	up_inner_link_ = NULL;
	down_inner_link_ = NULL;
	parent_ = NULL;
	invalidated_ = true;
	rejects_focus_ = false;
	hides_highlight_ = false;
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	z_order_ = 0;
	deletion_due_ = false;
	if(event_lock_)
	{
		pending_root_.push_back(this);
		pending_all_.push_back(this);
	} else
	{
		root_.push_back(this);
		all_.push_back(this);
	}
}

Widget::~Widget(void)
{
	root_.erase(std::remove(root_.begin(), root_.end(), this), root_.end());
	all_.erase(std::remove(all_.begin(), all_.end(), this), all_.end());
	pending_root_.erase(std::remove(pending_root_.begin(), pending_root_.end(), this), pending_root_.end());
	pending_all_.erase(std::remove(pending_all_.begin(), pending_all_.end(), this), pending_all_.end());

	if(widget_with_focus_ == this)
		widget_with_focus_ = NULL;
	for(vector<Widget*>::iterator it = children_.begin(); it != children_.end(); ++it)
	{
		(*it)->SetParent(NULL);	//Ensures that I don't try to act on a class executing it's destructor
		delete *it;
	}

	if(widget_with_modal_ == this)
		widget_with_modal_ = NULL;
	if(widget_with_depression_ == this)
		widget_with_depression_ = NULL;
	if(widget_with_highlight_ == this)
		widget_with_highlight_ = NULL;
	if(widget_with_modal_ == this)
		widget_with_modal_ = NULL;
	if(widget_with_drag_ == this)
		widget_with_drag_ = NULL;
	
	delete blit_rect_;
	delete back_rect_;
}
/* Complicated delete setup deserves an explanatory note.
   Problem is I cannot delete widgets from callbacks as they manipulate all_ and root_,
   invalidating the iterators. Instead widgets are marked for deletion, and then 
   automatically deleted once safe with the DeleteInternal method. This removes it from 
   any parent and frees it, and any children */
void Widget::Delete()
{
	if(event_lock_)
	{
		deletion_due_ = true;
	} else
		delete this;
}

void Widget::DeleteInternal()
{
	assert(deletion_due_);
	if(parent_)
		parent_->RemoveChild(this); //Puts this into root
	delete this;
}

vector<Widget*>& Widget::GetChildren()
{
	return children_;
}

void Widget::AddChild(Widget* _widget)
{
	if(event_lock_)
	{
		pending_root_.erase(std::remove(pending_root_.begin(), pending_root_.end(), _widget), pending_root_.end());
		pending_children_.push_back(_widget);
	} else
	{
		root_.erase(std::remove(root_.begin(), root_.end(), _widget), root_.end());
		children_.push_back(_widget);
		_widget->SetParent(this);
	}
	Invalidate();
}

/* Limitation, can only erase pending children while locked */
void Widget::RemoveChild(Widget* _widget)
{
	//TODO potential bug if widget removed from non parent
	if(event_lock_)
	{
		pending_root_.push_back(_widget);
		//pending_children_.erase(std::remove(pending_children_.begin(), pending_children_.end(), _widget), pending_children_.end());
		pending_removal_children_.push_back(_widget);
	} else
	{
		root_.push_back(_widget);
		children_.erase(std::remove(children_.begin(), children_.end(), _widget), children_.end());
	}	
	Invalidate();
}

void Widget::ClearChildren()
{
	std::vector<Widget*> detached = children_;
	detached.insert(detached.begin(), pending_children_.begin(), pending_children_.begin());
	children_.clear();
	pending_children_.clear();
	for(std::vector<Widget*>::iterator it = detached.begin(); it != detached.end(); ++it)
	{
		delete *it;
	}
	Invalidate();
}

void Widget::SetSize(Vector2i _size)
{
	delete blit_rect_;
	size_ = _size;
	blit_rect_ = new BlittableRect(size_);
	Invalidate();
}

void Widget::SetPosition(Vector2i _position)
{
	position_ = _position;
	if(parent_)
		parent_->Invalidate();
}

Widget* Widget::GetLeftParentLink()
{
	if(parent_)
	{
		Widget* p = parent_->GetLeftLink();
		if(p)
			return p;
		else
			return parent_->GetLeftParentLink();
	}
	return NULL;
}

Widget* Widget::GetRightParentLink()
{
	if(parent_)
	{
		Widget* p = parent_->GetRightLink();
		if(p)
			return p;
		else
			return parent_->GetRightParentLink();
	}
	return NULL;
}

Widget* Widget::GetUpParentLink()
{
	if(parent_)
	{
		Widget* p = parent_->GetUpLink();
		if(p)
			return p;
		else
			return parent_->GetUpParentLink();
	}
	return NULL;
}

Widget* Widget::GetDownParentLink()
{
	if(parent_)
	{
		Widget* p = parent_->GetDownLink();
		if(p)
			return p;
		else
			return parent_->GetDownParentLink();
	}
	return NULL;
}

void Widget::HandleEvent(Event _event)
{
	/* Mouse clicks */
	if((_event.event_type == EventType::MouseUp || _event.event_type == EventType::MouseMove || _event.event_type == EventType::MouseDown) &&
		(Collisions2i::PointInRectangle(Vector2i(_event.event.mouse_event.x, _event.event.mouse_event.y), Vector2i(0, 0), GetSize())))
	{
		if(_event.event_type == EventType::MouseUp || _event.event_type == EventType::MouseDown || _event.event_type == EventType::MouseMove)
		{
			for(vector<Widget*>::iterator it = children_.begin(); it != children_.end(); ++it)
			{
				//Transform event to child frame
				Event transformed_event = _event;
				transformed_event.event.mouse_event.x -= (*it)->GetPosition().x;
				transformed_event.event.mouse_event.y -= (*it)->GetPosition().y;
				if(Collisions2i::PointInRectangle(Vector2i(transformed_event.event.mouse_event.x, transformed_event.event.mouse_event.y), Vector2i(0,0), (*it)->GetSize()))
				{
					(*it)->HandleEvent(transformed_event);
					return;
				}
			}
		}
		if((!Widget::GetModalWidget() || HasOrInheritsModal()))
		{
			//No children consumed event, so parent consumes it 
			if(_event.event_type == EventType::MouseDown && _event.event.mouse_event.btns == MouseButton::Left)
			{
				if(_event.event.mouse_event.x < size_.x && _event.event.mouse_event.x >= 0 &&
				   _event.event.mouse_event.y < size_.y && _event.event.mouse_event.y >= 0)
				{
					SetDepresssed(true);
				}
			}
			if(_event.event_type == EventType::MouseUp && _event.event.mouse_event.btns == MouseButton::Left)
			{
				if(_event.event.mouse_event.x < size_.x && _event.event.mouse_event.x >= 0 &&
				   _event.event.mouse_event.y < size_.y && _event.event.mouse_event.y >= 0)
				{
					if(HasFocus())
					{
						OnFocusedClick(this);
					}
					SetDepresssed(false);
					SetFocus();
					OnClick(this);
					if(allow_edit_)
					{
						if(HasEditting())
						{
							SetEditting(false);
						} else
						{
							SetEditting(true);
						}

					}
				}
			}
			//All mouse buttons fire mouse click events, but only left gains focus
			if(_event.event_type == EventType::MouseUp && _event.event.mouse_event.btns != MouseButton::None)
			{
				if(_event.event.mouse_event.x < size_.x && _event.event.mouse_event.x >= 0 &&
				   _event.event.mouse_event.y < size_.y && _event.event.mouse_event.y >= 0)
				{
					MouseEventArgs e;
					e.x = _event.event.mouse_event.x;
					e.y = _event.event.mouse_event.y;
					e.btns = _event.event.mouse_event.btns;
					OnMouseClick(this, e);
				}
			}

			if(_event.event_type == EventType::MouseMove)
			{
				if(_event.event.mouse_event.x < size_.x && _event.event.mouse_event.x >= 0 &&
				   _event.event.mouse_event.y < size_.y && _event.event.mouse_event.y >= 0)
				{
					MouseEventArgs e;
					e.x = _event.event.mouse_event.x;
					e.y = _event.event.mouse_event.y;
					e.btns = _event.event.mouse_event.btns;
					OnMouseMove(this, e);
					SetHighlight();
					/* Handle start of drag and drop*/
					
				}				
			}
		}

	}
	/* Keyboard navigation */
	if(_event.event_type == EventType::KeyLeft)
	{
		if(this->left_inner_link_)
			left_inner_link_->SetFocus();
		else if(left_link_)
			left_link_->SetFocus();
		else if(GetLeftParentLink())
			GetLeftParentLink()->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyRight)
	{
		if(right_inner_link_)
			right_inner_link_->SetFocus();
		else if(right_link_)
			right_link_->SetFocus();
		else if(GetRightParentLink())
			GetRightParentLink()->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyUp)
	{
		if(up_inner_link_)
			up_inner_link_->SetFocus();
		else if(up_link_)
			up_link_->SetFocus();
		else if(GetUpParentLink())
			GetUpParentLink()->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyDown)
	{
		if(down_inner_link_)
			down_inner_link_->SetFocus();
		else if(down_link_)
			down_link_->SetFocus();
		else if(GetDownParentLink())
			GetDownParentLink()->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}

	if(_event.event_type == EventType::KeyEnter)
	{
		if(HasFocus()) //Possibly redundant, as will always have to have focus first
		{
			OnFocusedClick(this);
		}
		OnClick(this);
		if(HasEditting())
			SetEditting(false);
	}
	if(_event.event_type == EventType::KeyEscape)
	{
		if(HasEditting()) 
			SetEditting(false);
	}

	if(_event.event_type == EventType::OtherKeypress ||
		_event.event_type == EventType::KeyUp ||
		_event.event_type == EventType::KeyDown ||
		_event.event_type == EventType::KeyLeft ||
		_event.event_type == EventType::KeyRight ||
		_event.event_type == EventType::KeyEscape ||
		_event.event_type == EventType::KeyEnter)
	{
		KeyPressEventArgs args;
		args.key_code = _event.event.key_event.key_code;
		OnKeyUp(this, args);
	}

	if(_event.event_type == EventType::OtherKeypress && _event.event.key_event.key_up && HasEditting())
	{
		//Expecting ascii
		if(_event.event.key_event.key_code >= 32 && 
		   _event.event.key_event.key_code <= 127)
		{
			char nc = _event.event.key_event.key_code;
			if(_event.event.key_event.key_code >= 65 && _event.event.key_event.key_code <= 90)
				nc = _event.event.key_event.key_code + ((!_event.event.key_event.shift) ? 32 : 0);
			if(_event.event.key_event.key_code >= 97 && _event.event.key_event.key_code <= 122)
				nc = _event.event.key_event.key_code + (_event.event.key_event.shift ? -32 : 0);
			std::string cur_text = widget_text_.GetText();
			cur_text = cur_text + nc;
			widget_text_.SetText(cur_text);
			Invalidate();
		}
		if(_event.event.key_event.key_code == 8) //Backspace
		{
			if(widget_text_.GetText().length() > 0)
			{
				widget_text_.SetText(widget_text_.GetText().substr(0, widget_text_.GetText().length() - 1));
				Invalidate();
			}
		}
	}
}

void Widget::SetText(std::string _text, TextAlignment::Enum _alignment)
{
	bool change = false;

	change = widget_text_.GetText() != _text || widget_text_.GetAlignment() != _alignment;

	widget_text_.SetAlignment(_alignment);
	widget_text_.SetText(_text);

	if(change)
		Invalidate();
}
void Widget::SetTextWrap(bool _wrap)
{
	bool change = false;
	change = widget_text_.GetAutowrap() != _wrap;

	if(change)
	{
		widget_text_.SetAutowrap(_wrap, size_.x / 16);
		Invalidate();
	}
}


void Widget::Redraw()
{
	//Draw self - puts backbuffer onto front buffer. Use raw blit to copy alpha
	back_rect_->RawBlit(Vector2i(0,0), blit_rect_);
	//Superimpose text
	blit_rect_->BlitTextLines(widget_text_.GetTextLines(), widget_text_.GetAlignment());
	//Do any custom hooked drawing
	OnDraw(this, blit_rect_);

	if(widget_with_focus_ == this && !hides_highlight_)
		blit_rect_->Fade(0.25f, 255, 255, 255);
	if(widget_with_highlight_ == this && !hides_highlight_)
		blit_rect_->Fade(0.10f, 0, 0, 255);
	if(GetModalWidget() && !HasOrInheritsModal())
		blit_rect_->Fade(0.5f, 0, 0, 0);

	//Sort children by z order
	std::sort(children_.begin(), children_.end(), WidgetZSort<Widget*>());
	//Blit in children
	for(vector<Widget*>::iterator it = children_.begin(); it != children_.end(); ++it)
	{
		if((*it)->invalidated_)
			(*it)->Redraw();
		if((*it)->GetVisibility())
		{
			if((*it)->ignore_dest_transparency_)
			{
				(*it)->blit_rect_->RawBlit((*it)->GetPosition(), blit_rect_);
			} else
				(*it)->blit_rect_->Blit((*it)->GetPosition(), blit_rect_);
		}
	}
	invalidated_ = false;
}

void Widget::SetFocus()
{
	if(rejects_focus_ || (GetModalWidget() && !HasOrInheritsModal()))
		return;
	if(Widget::widget_with_focus_ != this)
	{
		OnGainFocus(this);
		Widget* pOldWidgetWithFocus = Widget::widget_with_focus_;
		Widget::widget_with_focus_ = this;
		if(pOldWidgetWithFocus)
		{
			pOldWidgetWithFocus->OnLostFocus(this);
			pOldWidgetWithFocus->Invalidate();
		}

		if(Widget::widget_with_edit_ != this)
		{
			widget_with_edit_ = NULL;
		}
	}
	
	Invalidate();
}

void Widget::ClearFocus()
{
	if(widget_with_focus_)
	{
		Widget* old_widget_with_focus = widget_with_focus_;
		widget_with_focus_->OnLostFocus(widget_with_focus_);
		widget_with_focus_ = NULL;
		old_widget_with_focus->Invalidate();
	}
}

void Widget::SetHighlight()
{
	if(Widget::widget_with_highlight_ != this && widget_with_drag_ != NULL)
	{
		
		if(OnDragEnter.num_slots() > 0)
		{
			//DragEventArgs dea2 = drag_event_args_;
			//dea2.drag_accept
			drag_event_args_.drag_accepted = false;
			OnDragEnter(this, &drag_event_args_);
		}
		
	}

	if(rejects_focus_)
		return;
	if(Widget::widget_with_highlight_ != this)
	{
		Widget* pOldWidgetWithHighlight = Widget::widget_with_highlight_;
		Widget::widget_with_highlight_ = this;
		if(pOldWidgetWithHighlight)
		{
			pOldWidgetWithHighlight->Invalidate();
		}
	}

	Invalidate();
}

void Widget::SetDepresssed(bool _depressed)
{
	if(_depressed)
	{
		depressed_ = true;
		Invalidate();

		Widget* pOldWigetWithDepression = Widget::widget_with_depression_;
		if(_depressed)
			Widget::widget_with_depression_	= this;		
		if(pOldWigetWithDepression && pOldWigetWithDepression != this)
		{
			pOldWigetWithDepression->depressed_ = false;
			pOldWigetWithDepression->Invalidate();
		}
		
	} else
	{
		depressed_ = false;
		if(Widget::widget_with_depression_ == this)
			Widget::widget_with_depression_	= NULL;
	}

}

void Widget::SetModal(bool _modal)
{
	Widget* old_modal_widget = widget_with_modal_;

	if(_modal)
		widget_with_modal_ = this;
	else if(widget_with_modal_ == this)
		widget_with_modal_ = NULL;
	if(_modal)
		Widget::ClearFocus();
	if(old_modal_widget && old_modal_widget != widget_with_modal_)
	{
		old_modal_widget->SetZOrder(old_modal_widget->GetZOrder() - 100000);
	}
	if(widget_with_modal_ != old_modal_widget && _modal)
		SetZOrder(GetZOrder() + 100000);
	if(widget_with_modal_ != old_modal_widget)
	{
		for(std::vector<Widget*>::iterator it = all_.begin(); it != all_.end(); ++it)
		{
			(*it)->Invalidate();
		}
	}
}

void Widget::SetModalWidget(Widget* _widget)
{
	_widget->SetModal(true);
}

bool Widget::HasOrInheritsModal()
{
	if(parent_)
		return parent_->HasModal() || HasModal();
	else
		return HasModal();
}

void Widget::Invalidate()
{
	if(parent_)
		parent_->Invalidate();
	invalidated_ = true;
}

void Widget::InsertPending()
{
	for(vector<Widget*>::iterator it = pending_removal_children_.begin(); it != pending_removal_children_.end(); ++it)
	{
		children_.erase(std::remove(children_.begin(), children_.end(), *it), children_.end());
	}
	
	pending_removal_children_.clear();
	children_.insert(children_.end(), pending_children_.begin(), pending_children_.end());
	for(vector<Widget*>::iterator it = pending_children_.begin(); it != pending_children_.end(); ++it)
	{
		(*it)->SetParent(this);
	}
	pending_children_.clear();
}


void Widget::ClearRoot()
{
	vector<Widget*> root_copy = root_; //Make a copy - ~Widget removes itself from the list, invalidating iterators etc
	root_copy.insert(root_copy.end(), pending_root_.begin(), pending_root_.end());
	for(vector<Widget*>::iterator it = root_copy.begin(); it != root_copy.end(); ++it)
	{
		if(event_lock_)
			(*it)->Delete();
		else
			delete (*it);
	}
	assert(root_.size() == 0);
	widget_with_focus_ = NULL;
	widget_with_highlight_ = NULL;
	widget_with_depression_ = NULL;
	widget_with_drag_ = NULL;
	widget_with_modal_ = NULL;
	widget_with_edit_ = NULL;
	drag_event_args_ = DragEventArgs();

	//root_.clear(); // The destructors do this automatically
}

void Widget::RenderRoot(BlittableRect* _screen)
{
	std::sort(root_.begin(), root_.end(), WidgetZSort<Widget*>());
	for(vector<Widget*>::iterator it = root_.begin(); it != root_.end(); ++it)
	{
		if((*it)->invalidated_)
		{
			(*it)->Redraw();
		}
		if((*it)->GetVisibility())
			(*it)->blit_rect_->Blit((*it)->GetPosition(), _screen);
	}
	if(screen_fade_rect_ == NULL || screen_fade_rect_->GetSize() != _screen->GetSize())
	{
		delete screen_fade_rect_;
		screen_fade_rect_ = new BlittableRect(_screen->GetSize());
		screen_fade_rect_->Fill(static_cast<unsigned char>(screen_fade_ * 255), 0, 0, 0);	
	}
	if(screen_fade_ > 0)
	{
		screen_fade_rect_->Blit(Vector2i(0, 0), _screen);
	}

	if(edit_cursor_rect_ == NULL)
	{
		edit_cursor_rect_ = new BlittableRect("TextCursor.png");
	}
	if(widget_with_edit_)
	{
		if(fmod(sum_time_, 0.5) < 0.25)
		{
			Vector2i top_left, bottom_right;
			widget_with_edit_->blit_rect_->MeasureText(widget_with_edit_->widget_text_.GetText(), widget_with_edit_->widget_text_.GetAlignment(), top_left, bottom_right);
			edit_cursor_rect_->Blit(widget_with_edit_->GetGlobalPosition() + Vector2i(bottom_right.x, bottom_right.y), _screen);
		}
		
	}

}

void Widget::DistributeSDLEvents(SDL_Event* event)
{
	event_lock_ = true;
	Event e;
	if(event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEBUTTONDOWN)
	{
		e.event.mouse_event.x = event->button.x;
		e.event.mouse_event.y = event->button.y;
		e.event.mouse_event.btns = (MouseButton::Enum)(((event->button.button == SDL_BUTTON_LEFT) ? MouseButton::Left : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_RIGHT) ? MouseButton::Right : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_MIDDLE) ? MouseButton::Middle : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELUP) ? MouseButton::ScrollUp : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELDOWN) ? MouseButton::ScrollDown : MouseButton::None));
		if(event->type == SDL_MOUSEBUTTONDOWN)
			e.event_type = EventType::MouseDown;
		else
			e.event_type = EventType::MouseUp;
	} else if(event->type == SDL_KEYUP || event->type == SDL_KEYDOWN)
	{
		e.event.key_event.key_code = event->key.keysym.sym;
		if(event->type == SDL_KEYUP)
		{
			e.event.key_event.key_up = true;
		} else
		{
			e.event.key_event.key_up = false;
		}


		
		if(event->key.keysym.sym == SDLK_LEFT)
		{
			e.event_type = EventType::KeyLeft;
		}
		else if(event->key.keysym.sym == SDLK_RIGHT)
		{
			e.event_type = EventType::KeyRight;
		}
		else if(event->key.keysym.sym == SDLK_UP)
		{
			e.event_type = EventType::KeyUp;
		}
		else if(event->key.keysym.sym == SDLK_DOWN)
		{
			e.event_type = EventType::KeyDown;
		}
		else if(event->key.keysym.sym == SDLK_RETURN)
		{
			e.event_type = EventType::KeyEnter;
		}
		else if(event->key.keysym.sym == SDLK_ESCAPE)
		{
			e.event_type = EventType::KeyEscape;
		} else
		{
			e.event_type = EventType::OtherKeypress;
		}
		e.event.key_event.shift = SDL_GetModState() & (KMOD_RSHIFT | KMOD_LSHIFT);
	} else if(event->type == SDL_MOUSEMOTION)
	{
		e.event_type = EventType::MouseMove;
		e.event.mouse_event.x = event->motion.x;
		e.event.mouse_event.y = event->motion.y;
		e.event.mouse_event.btns = MouseButton::None;
	}else
	{
		RemoveEventLock();
		return;
	}

	/* End mouse dragging */
	if(e.event_type == EventType::MouseUp && e.event.mouse_event.btns == MouseButton::Left &&
	   widget_with_drag_ != NULL)
	{
		if(widget_with_highlight_ != NULL && widget_with_drag_ != widget_with_highlight_ && drag_event_args_.drag_accepted)
		{
			widget_with_highlight_->OnDragLand(widget_with_highlight_, &drag_event_args_);
		} else
		{
			drag_event_args_.x = e.event.mouse_event.x - drag_start_position_.x;
			drag_event_args_.y = e.event.mouse_event.y - drag_start_position_.y;
			drag_event_args_.sx = drag_start_position_.x - widget_with_drag_->GetPosition().x;
			drag_event_args_.sy = drag_start_position_.y - widget_with_drag_->GetPosition().y;
			drag_event_args_.ex = e.event.mouse_event.x - widget_with_drag_->GetPosition().x;
			drag_event_args_.ey = e.event.mouse_event.y - widget_with_drag_->GetPosition().y;
			widget_with_drag_->OnDragReset(widget_with_drag_, &drag_event_args_);
		}

		widget_with_drag_ = NULL;
		widget_with_depression_ = NULL;

		RemoveEventLock();
		return;
	}

	
	if(e.event_type == EventType::MouseUp || e.event_type == EventType::MouseDown)
	{
		for(vector<Widget*>::iterator it = root_.begin(); it != root_.end(); ++it)
		{
			Event e2 = e;
			e2.event.mouse_event.x -= (*it)->GetPosition().x;
			e2.event.mouse_event.y -= (*it)->GetPosition().y;
			(*it)->HandleEvent(e2);
		}
	}

	/* Start mouse drag */
	if(e.event_type == EventType::MouseMove)
	{
		if(widget_with_depression_ != NULL && widget_with_depression_->allow_drag_ && widget_with_drag_ == NULL)
		{
			DragEventArgs dea;
			widget_with_depression_->OnDragStart(widget_with_depression_, &dea);
			if(dea.drag_type != 0)
			{
				widget_with_drag_ = widget_with_depression_;
				drag_event_args_ = dea;
				drag_start_position_ = Vector2i(e.event.mouse_event.x, e.event.mouse_event.y);
				//drag_start_position_ += widget_with_depression_->GetGlobalPosition();
			}
		}
	}

	if(e.event_type == EventType::MouseMove)
	{
		for(vector<Widget*>::iterator it = all_.begin(); it != all_.end(); ++it)
		{
			Event e2 = e;
			e2.event.mouse_event.x -= (*it)->GetGlobalPosition().x;
			e2.event.mouse_event.y -= (*it)->GetGlobalPosition().y;
			(*it)->HandleEvent(e2);
		}
	}


	if(e.event_type == EventType::KeyDown ||
		e.event_type == EventType::KeyUp ||
		e.event_type == EventType::KeyLeft ||
		e.event_type == EventType::KeyRight ||
		e.event_type == EventType::KeyEnter ||
		e.event_type == EventType::KeyEscape ||
		e.event_type == EventType::OtherKeypress)
	{
		Widget* focus = Widget::GetWidgetWithFocus();
		if(focus)
			focus->HandleEvent(e);

		KeyPressEventArgs kp_args;
		kp_args.key_code = e.event.key_event.key_code;
		kp_args.key_up = e.event.key_event.key_up;
		Widget::OnGlobalKeyUp(NULL, kp_args);
	}
	RemoveEventLock();
}

void Widget::RemoveEventLock()
{
	event_lock_ = false;

	/* Merge widgets created during this callback */
	all_.insert(all_.end(), pending_all_.begin(), pending_all_.end());
	pending_all_.clear();



	for(std::vector<Widget*>::iterator it = all_.begin(); it != all_.end(); ++it)
	{
		(*it)->InsertPending();
	}	
	root_.insert(root_.end(), pending_root_.begin(), pending_root_.end());
	pending_root_.clear();

	vector<Widget*> all_copy = all_;
	for(vector<Widget*>::iterator it = all_copy.begin(); it != all_copy.end(); ++it)
	{
		if((*it)->deletion_due_)
			(*it)->DeleteInternal();
	}
}

Vector2i Widget::GetGlobalPosition()
{
	if(parent_)
	{
		return position_ + parent_->GetGlobalPosition();
	} else
	{
		return position_;
	}
}

void Widget::SetFade(float _fade_amount)
{
	_fade_amount = _fade_amount < 0.0f ? 0.0f : _fade_amount > 1.0f ? 1.0f : _fade_amount;
	screen_fade_ = _fade_amount;
	if(screen_fade_rect_)
		screen_fade_rect_->Fill(static_cast<unsigned char>(screen_fade_ * 255), 0, 0, 0);	
}

bool Widget::HasEditting()
{
	return widget_with_edit_ == this;
}

void Widget::SetEditting(bool _editting)
{
	if(_editting && allow_edit_)
		widget_with_edit_ = this;
	else if(widget_with_edit_ == this)
		widget_with_edit_ = NULL;
}