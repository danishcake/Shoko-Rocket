#pragma once
#include "Widget.h"

class ItemBrowserWidget :
	public Widget
{
protected:
	Vector2i grid_size_;
	Vector2i item_size_;
	vector<std::string> items_;
	vector<Widget*> clickable_items_;
	int page_;

	void ItemClick(Widget* _widget);
	void ItemFocusedClick(Widget* _widget);
	
	Widget* last_clicked_;
public:
	/* Typedefs */
	typedef boost::signal<void (Widget*, BlittableRect**, std::string)> ItemRenderEvent;
	typedef boost::signal<void (Widget*, std::string)> ItemClickEvent;
	typedef boost::signal<void (Widget*, int, int)> PageChangeEvent;

	/* Constructors */
	ItemBrowserWidget(vector<std::string> _items, Vector2i _grid_size, Vector2i _item_size);
	virtual ~ItemBrowserWidget(void);

	/* Properties */
	vector<std::string> GetItems(){return items_;}
	void SetItems(vector<std::string> _items);
	Vector2i GetGridSize(){return grid_size_;}
	Vector2i GetItemSize(){return item_size_;}
	int GetPage(){return page_;}
	int GetPageCount();
	

	/* Events */
	ItemRenderEvent OnItemRender;
	ItemClickEvent OnItemClick;
	ItemClickEvent OnItemSelectedChanged;
	ItemClickEvent OnItemFocusedClick;
	PageChangeEvent OnPageChange;
	
	/* Layout methods */
	void PerformItemLayout();

	/* Event handlers */
	/* Can also be called with NULL to just advance page */
	void NextPage(Widget* _widget);
	void PrevPage(Widget* _widget);
	void SetPage(int _page);

};
