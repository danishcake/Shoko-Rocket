#pragma once
#include "Widget.h"

class MenuWidget :
	public Widget
{
public:
	MenuWidget(void);
	virtual ~MenuWidget(void);

	Widget* AddMenuItem(std::string _item);
	
	void SetFocusToFirst();
};
