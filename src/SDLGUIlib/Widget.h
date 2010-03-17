#pragma once
#include "vmath.h"
#include <vector>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include "Event.h"
#include "TextAlignment.h"
#include "BlittableRect.h"
#include "Tiling.h"
#include <sdl.h>

//Widgets should not be used on the stack - they are tracked automatically!
using std::vector;

class Widget
{
protected:
	Vector2i position_;
	Vector2i size_;
	vector<Widget*> children_;
	vector<Widget*> pending_children_;
	vector<Widget*> pending_removal_children_;
	Widget* parent_;
	Widget* left_link_;
	Widget* right_link_;
	Widget* up_link_;
	Widget* down_link_;
	Widget* left_inner_link_;
	Widget* right_inner_link_;
	Widget* up_inner_link_;
	Widget* down_inner_link_;
	BlittableRect* blit_rect_;
	BlittableRect* back_rect_;
	int z_order_;
	bool deletion_due_;

	static Widget* widget_with_focus_;
	static Widget* widget_with_highlight_;
	static Widget* widget_with_depression_;
	static Widget* widget_with_drag_;
	static Widget* widget_with_modal_;
	static Widget* widget_with_edit_;

	static DragEventArgs drag_event_args_;
	static Vector2i drag_start_position_;

	static vector<Widget*> root_; // The widgets that aren't children
	static vector<Widget*> all_;  // All the widgets
	static vector<Widget*> pending_root_; //Pending version are widgets added by a callback
	static vector<Widget*> pending_all_;
	static bool event_lock_;

	bool invalidated_;
	bool rejects_focus_;
	bool hides_highlight_; //For item browser widget to prevent background turning blue
	bool allow_drag_;
	bool depressed_;
	bool visible_;
	bool ignore_dest_transparency_;
	bool allow_edit_;

	WidgetText widget_text_;
	TextSize::Enum font_size_;
	std::string tag_;			//Stores some related name or useful data

	static float screen_fade_;
	static BlittableRect* screen_fade_rect_;
	static BlittableRect* edit_cursor_rect_;

	static double sum_time_;
	void InsertPending();
	void DeleteInternal();
	static void RemoveEventLock();
	static bool InheritsDeleteDue(Widget* _widget);
public:
	/* Typedefs etc */
	typedef boost::signal<void (Widget*)> WidgetEvent;
	typedef boost::signal<void (Widget*, MouseEventArgs)> MouseEvent;
	typedef boost::signal<void (Widget*, KeyPressEventArgs)> KeyEvent;
	typedef boost::signal<void (Widget*, DragEventArgs*)> DragEvent;
	typedef boost::signal<void (Widget*, BlittableRect*)> DrawEvent;

	/* Constructors */
	Widget(void);
	Widget(std::string _filename);
	Widget(BlittableRect* _blittable);
	Widget(VerticalTile _tiles, int _height);
	Widget(HorizontalTile _tiles, int _width);
	virtual ~Widget(void);
	void Delete();
	static Widget* GetWidgetWithFocus(){return widget_with_focus_;}

	/* Getters and setters */
	Vector2i GetPosition(){return position_;}
	Vector2i GetGlobalPosition();
	void SetPosition(Vector2i _position);
	Vector2i GetSize(){return size_;}
	void SetSize(Vector2i _size);
	void SetZOrder(int _z_order){z_order_ = _z_order;}
	int GetZOrder(){return z_order_;}
	void SetTag(std::string _tag){tag_ = _tag;}
	std::string GetTag(){return tag_;}

	/* Children members */
	vector<Widget*>& GetChildren();
	void AddChild(Widget* _widget);
	void RemoveChild(Widget* _widget);
	void ClearChildren();
	/* Parent members */
	void SetParent(Widget* _widget){parent_ = _widget;}
	Widget* GetParent(){return parent_;}

	/* Signals */
	WidgetEvent OnClick;
	WidgetEvent OnFocusedClick;
	MouseEvent  OnMouseClick;
	MouseEvent  OnMouseMove;
	WidgetEvent OnGainFocus;
	WidgetEvent OnLostFocus;
	WidgetEvent OnEditFinish;
	KeyEvent    OnKeyUp;
	DrawEvent   OnDraw;
	
	/* Static signals */
	static KeyEvent OnGlobalKeyUp;

	DragEvent   OnDragStart; //Allows filling of drag data - drag_type must be non zero
	DragEvent   OnDragReset;
	DragEvent   OnDragEnter;
	DragEvent   OnDragLand;

	/* Event handling */
	void HandleEvent(Event _event);

	/* Keyboard links */
	void LinkLeft(Widget* _link){left_link_ = _link;}
	void LinkRight(Widget* _link){right_link_ = _link;}
	void LinkUp(Widget* _link){up_link_ = _link;}
	void LinkDown(Widget* _link){down_link_ = _link;}

	Widget* GetLeftLink(){return left_link_;}
	Widget* GetRightLink(){return right_link_;}
	Widget* GetUpLink(){return up_link_;}
	Widget* GetDownLink(){return down_link_;}

	void LinkInnerLeft(Widget* _link){left_inner_link_ = _link;}
	void LinkInnerRight(Widget* _link){right_inner_link_ = _link;}
	void LinkInnerUp(Widget* _link){up_inner_link_ = _link;}
	void LinkInnerDown(Widget* _link){down_inner_link_ = _link;}
	
	Widget* GetLeftParentLink();
	Widget* GetRightParentLink();
	Widget* GetUpParentLink();
	Widget* GetDownParentLink();

	/* Visibility */
	bool GetVisibility(){return visible_;}
	void SetVisibility(bool _visibility){visible_ = _visibility;}

	/* Focus */
	bool HasFocus(){return widget_with_focus_ == this;}
	void SetFocus();
	static void ClearFocus();
	void SetRejectsFocus(bool _focus){rejects_focus_ = _focus;}
	bool GetRejectsFocus(){return rejects_focus_;}

	/* Highlight */
	bool HasHighlight(){return widget_with_highlight_ == this;}
	void SetHidesHighlight(bool _hides_highlight){hides_highlight_ = _hides_highlight;}
	void SetHighlight();

	/* Click depression */
	bool HasDepressed(){return depressed_;}
	void SetDepresssed(bool _depressed);

	/* Drag and drop */
	bool GetAllowDrag(){return allow_drag_;}
	void SetAllowDrag(bool _allow_drag){allow_drag_ = _allow_drag;}
	static Widget* GetDraggedWidget(){return widget_with_drag_;}

	/* Drawing and redrawing */
	void Redraw();
	void Invalidate();
	void SetText(std::string _text, TextAlignment::Enum _alignment);
	void SetTextWrap(bool _wrap);
	BlittableRect* GetBackRect(){return back_rect_;}


	/* Root handling and drawing */
	static void ClearRoot();
	static vector<Widget*> GetRoot(){return root_;}
	static void RenderRoot(BlittableRect* _screen);
	static void DistributeSDLEvents(SDL_Event* event);
	static void Tick(float _dt){sum_time_ += _dt;}	

	/* Modal widget */
	static Widget* GetModalWidget(){return widget_with_modal_;}
	static void SetModalWidget(Widget* _widget);
	bool HasModal(){return widget_with_modal_ == this;}
	bool HasOrInheritsModal();
	void SetModal(bool _modal);

	/* Text edit */
	static Widget* GetEdittingWidget(){return widget_with_edit_;}
	bool GetEditable(){return allow_edit_;}
	void SetEditable(bool _editable){allow_edit_ = _editable;}
	bool HasEditting();
	void SetEditting(bool _editting);
	std::string GetText(){return widget_text_.GetText();}
	void SetTextSize(TextSize::Enum _text_size){font_size_ = _text_size;}
	

	/* Screen fading */
	static void SetFade(float _fade_amount);
	static float GetFade(){return screen_fade_;}
};
