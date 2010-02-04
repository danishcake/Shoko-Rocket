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
DragEventArgs Widget::drag_event_args_ = DragEventArgs();
Vector2i Widget::drag_start_position_ = Vector2i(0, 0);

vector<Widget*> Widget::root_ = vector<Widget*>();
vector<Widget*> Widget::all_ = vector<Widget*>();
float Widget::screen_fade_ = 0.0f;
BlittableRect* Widget::screen_fade_rect_ = NULL;

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
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	root_.push_back(this);
	all_.push_back(this);
	
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
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	root_.push_back(this);
	all_.push_back(this);
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
	allow_drag_ = false;
	depressed_ = false;
	ignore_dest_transparency_ = false;
	visible_ = true;
	allow_edit_ = false;
	root_.push_back(this);
	all_.push_back(this);
}

Widget::~Widget(void)
{
	root_.erase(std::remove(root_.begin(), root_.end(), this), root_.end());
	all_.erase(std::remove(all_.begin(), all_.end(), this), all_.end());
	if(widget_with_focus_ == this)
		widget_with_focus_ = NULL;
	for(vector<Widget*>::iterator it = children_.begin(); it != children_.end(); ++it)
	{
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

vector<Widget*>& Widget::GetChildren()
{
	return children_;
}

void Widget::AddChild(Widget* _widget)
{
	root_.erase(std::remove(root_.begin(), root_.end(), _widget), root_.end());
	children_.push_back(_widget);
	_widget->SetParent(this);
	Invalidate();
}

void Widget::RemoveChild(Widget* _widget)
{
	root_.push_back(_widget);
	children_.erase(std::remove(children_.begin(), children_.end(), _widget));
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
		if(_event.event_type == EventType::MouseUp || _event.event_type == EventType::MouseDown)
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
					SetHighlight();
					/* Handle start of drag and drop*/

				}
			}
		}

	}

	/* Keyboard navigation */
	Widget* parent_link = NULL;
	if(_event.event_type == EventType::KeyLeft)
	{
		if(this->left_inner_link_)
			left_inner_link_->SetFocus();
		else if(left_link_)
			left_link_->SetFocus();
		else if(parent_link = GetLeftParentLink())
			parent_link->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyRight)
	{
		if(right_inner_link_)
			right_inner_link_->SetFocus();
		else if(right_link_)
			right_link_->SetFocus();
		else if(parent_link = GetRightParentLink())
			parent_link->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyUp)
	{
		if(up_inner_link_)
			up_inner_link_->SetFocus();
		else if(up_link_)
			up_link_->SetFocus();
		else if(parent_link = GetUpParentLink())
			parent_link->SetFocus();
		else if(parent_)
			parent_->SetFocus();
	}
	if(_event.event_type == EventType::KeyDown)
	{
		if(down_inner_link_)
			down_inner_link_->SetFocus();
		else if(down_link_)
			down_link_->SetFocus();
		else if(parent_link = GetDownParentLink())
			parent_link->SetFocus();
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

	if(_event.event_type == EventType::OtherKeypress && HasEditting())
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
			widget_text_.text = widget_text_.text + nc;
		}
		if(_event.event.key_event.key_code == 8) //Backspace
		{
			if(widget_text_.text.length() > 0)
				widget_text_.text = widget_text_.text.substr(0, widget_text_.text.length() - 1);
		}
	}
}

void Widget::SetText(std::string _text, TextAlignment::Enum _alignment)
{
	bool change = false;
	change = widget_text_.text != _text || widget_text_.alignment != _alignment;

	widget_text_.alignment = _alignment;
	widget_text_.text = _text;

	if(change)
		Invalidate();
}

void Widget::Redraw()
{
	//Draw self - puts backbuffer onto front buffer. Use raw blit to copy alpha
	back_rect_->RawBlit(Vector2i(0,0), blit_rect_);
	//Superimpose text
	blit_rect_->BlitText(widget_text_.text, widget_text_.alignment);
	if(widget_with_focus_ == this)
		blit_rect_->Fade(0.25f, 255, 255, 255);
	if(widget_with_highlight_ == this)
		blit_rect_->Fade(0.10f, 0, 0, 255);
	if(GetModalWidget() && !HasOrInheritsModal())
		blit_rect_->Fade(0.5f, 0, 0, 0);

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
	if(Widget::widget_with_highlight_ != this)
	{
		drag_event_args_.drag_accepted = false;
		OnDragEnter(this, &drag_event_args_);
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
	Widget* old_modal_widget = widget_with_modal_;

	widget_with_modal_ = _widget;
	if(_widget) 
		Widget::ClearFocus();

	if(widget_with_modal_ != old_modal_widget)
	{
		for(std::vector<Widget*>::iterator it = all_.begin(); it != all_.end(); ++it)
		{
			(*it)->Invalidate();
		}
	}
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

void Widget::ClearRoot()
{
	vector<Widget*> root_copy = root_; //Make a copy - ~Widget removes itself from the list, invalidating iterators etc
	for(vector<Widget*>::iterator it = root_copy.begin(); it != root_copy.end(); ++it)
	{
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
		screen_fade_rect_->Fill(screen_fade_ * 255, 0, 0, 0);	
	}

	if(screen_fade_ > 0)
	{
		screen_fade_rect_->Blit(Vector2i(0, 0), _screen);
	}
}

void Widget::DistributeSDLEvents(SDL_Event* event)
{
	Event e;
	if(event->type == SDL_MOUSEBUTTONUP)
	{
		e.event.mouse_event.x = event->button.x;
		e.event.mouse_event.y = event->button.y;
		e.event.mouse_event.btns = (MouseButton::Enum)(((event->button.button == SDL_BUTTON_LEFT) ? MouseButton::Left : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_RIGHT) ? MouseButton::Right : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_MIDDLE) ? MouseButton::Middle : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELUP) ? MouseButton::ScrollUp : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELDOWN) ? MouseButton::ScrollDown : MouseButton::None));
		e.event_type = EventType::MouseUp;
	}  else if(event->type == SDL_MOUSEBUTTONDOWN)
	{
		e.event.mouse_event.x = event->button.x;
		e.event.mouse_event.y = event->button.y;
		e.event.mouse_event.btns = (MouseButton::Enum)(((event->button.button == SDL_BUTTON_LEFT) ? MouseButton::Left : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_RIGHT) ? MouseButton::Right : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_MIDDLE) ? MouseButton::Middle : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELUP) ? MouseButton::ScrollUp : MouseButton::None) |
													   ((event->button.button == SDL_BUTTON_WHEELDOWN) ? MouseButton::ScrollDown : MouseButton::None));
		e.event_type = EventType::MouseDown;
	} else if(event->type == SDL_KEYUP)
	{
		if(event->key.keysym.sym == SDLK_LEFT)
		{
			e.event_type = EventType::KeyLeft;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else if(event->key.keysym.sym == SDLK_RIGHT)
		{
			e.event_type = EventType::KeyRight;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else if(event->key.keysym.sym == SDLK_UP)
		{
			e.event_type = EventType::KeyUp;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else if(event->key.keysym.sym == SDLK_DOWN)
		{
			e.event_type = EventType::KeyDown;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else if(event->key.keysym.sym == SDLK_RETURN)
		{
			e.event_type = EventType::KeyEnter;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else if(event->key.keysym.sym == SDLK_ESCAPE)
		{
			e.event_type = EventType::KeyEscape;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		else
		{
			e.event_type = EventType::OtherKeypress;
			e.event.key_event.key_code = event->key.keysym.sym;
		}
		e.event.key_event.shift = SDL_GetModState() & (KMOD_RSHIFT | KMOD_LSHIFT);
	} else if(event->type == SDL_MOUSEMOTION)
	{
		e.event_type = EventType::MouseMove;
		e.event.mouse_event.x = event->motion.x;
		e.event.mouse_event.y = event->motion.y;
		e.event.mouse_event.btns = MouseButton::None;
	}else
		return;

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
			widget_with_drag_->OnDragReset(widget_with_drag_, &drag_event_args_);
		}

		widget_with_drag_ = NULL;
		widget_with_depression_ = NULL;

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
		screen_fade_rect_->Fill(screen_fade_ * 255, 0, 0, 0);	
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