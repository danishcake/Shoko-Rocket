#include "Logger.h"
#include "MenuWidget.h"

MenuWidget::MenuWidget(void)
{
	rejects_focus_ = true;
}

MenuWidget::~MenuWidget(void)
{
}

Widget*  MenuWidget::AddMenuItem(std::string _item)
{
	Widget* pMenuItem = new Widget();
	pMenuItem->SetSize(Vector2i(128, 48));
	pMenuItem->SetPosition(Vector2i(static_cast<int>(children_.size()) * 8, static_cast<int>(children_.size()) * 40));
	pMenuItem->SetText(_item, TextAlignment::Centre);
	SetSize(Vector2i(128 + static_cast<int>(children_.size()) * 8, 48 + static_cast<int>(children_.size()) * 40));
	//Link up
	if(children_.size() == 1)
	{
		children_.at(0)->LinkUp(pMenuItem);
		children_.at(0)->LinkDown(pMenuItem);
	}
	if(children_.size() > 0)
	{
		pMenuItem->LinkUp(children_.at(children_.size() - 1));
		pMenuItem->LinkDown(children_.at(0));
	}
	//Remove old link
	if(children_.size() > 1)
	{
		children_.at(children_.size() - 1)->LinkDown(pMenuItem);
		children_.at(0)->LinkUp(pMenuItem);
	}

	AddChild(pMenuItem);

	return pMenuItem;
}

void MenuWidget::SetFocusToFirst()
{
	if(children_.size() > 0)
		children_.at(0)->SetFocus();
}