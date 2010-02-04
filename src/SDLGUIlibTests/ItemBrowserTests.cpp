#include "stdafx.h"
#include "ItemBrowserWidget.h"

using std::vector;
using std::string;

TEST_FIXTURE(SDL_fixture, ItemBrowserTakesListOfFiles)
{
	UNITTEST_TIME_CONSTRAINT(50);

 	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		vector<string> files;
		files.push_back("1");
		files.push_back("2");
		files.push_back("3");
		files.push_back("4");
		files.push_back("5");
		files.push_back("6");
		files.push_back("7");
		ItemBrowserWidget* ibw = new ItemBrowserWidget(files, Vector2i(2, 2), Vector2i(36, 27));

		delete ibw;
	}
}

TEST_FIXTURE(SDL_fixture, ItemBrowserChildren)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		vector<string> files;
		files.push_back("1");
		files.push_back("2");
		files.push_back("3");
		files.push_back("4");
		files.push_back("5");
		files.push_back("6");
		files.push_back("7");
		ItemBrowserWidget* ibw = new ItemBrowserWidget(files, Vector2i(2, 2), Vector2i(36, 27));

		CHECK_EQUAL(4, ibw->GetChildren().size()); // 2x1 + 2

		delete ibw;
	}
}

TEST_FIXTURE(SDL_fixture, ItemBrowerPages)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		vector<string> files;
		files.push_back("1");
		files.push_back("2");
		files.push_back("3");
		files.push_back("4");
		files.push_back("5");
		files.push_back("6");
		files.push_back("7");
		ItemBrowserWidget* ibw = new ItemBrowserWidget(files, Vector2i(3, 2), Vector2i(36, 27));
		//3x2 - 2 = 4 -> 1 full page, 1 page 3/4

		CHECK_EQUAL(6, ibw->GetChildren().size());
		CHECK_EQUAL(0, ibw->GetPage());

		ibw->NextPage(NULL);
		CHECK_EQUAL(5, ibw->GetChildren().size());
		CHECK_EQUAL(1, ibw->GetPage());

		ibw->NextPage(NULL);
		CHECK_EQUAL(6, ibw->GetChildren().size());
		CHECK_EQUAL(0, ibw->GetPage());

		ibw->PrevPage(NULL);
		CHECK_EQUAL(5, ibw->GetChildren().size());
		CHECK_EQUAL(1, ibw->GetPage());

		delete ibw;
	}
	
}

TEST_FIXTURE(SDL_fixture, ItemBrowserEmpty)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		vector<string> files;
		ItemBrowserWidget* ibw = new ItemBrowserWidget(files, Vector2i(3, 2), Vector2i(36, 27));
		CHECK_EQUAL(1, ibw->GetPageCount());
	}
}

namespace
{
	int item_click_count = 0;
	std::string last_item_clicked;
	void ItemClickHandler(Widget* /*_widget*/, std::string _item)
	{
		item_click_count++;
		last_item_clicked = _item;
	}

	int item_selected_change_count  = 0;
	std::string last_item_selected;
	void ItemSelectionHandler(Widget* /*_widget*/, std::string _item)
	{
		item_selected_change_count++;
		last_item_selected = _item;
	}

	int item_focus_click_count  = 0;
	std::string last_focus_item;
	void ItemFocusedClickHandler(Widget* /*_widget*/, std::string _item)
	{
		item_focus_click_count++;
		last_focus_item = _item;
	}
}


TEST_FIXTURE(SDL_fixture, ItemBrowserEvents)
{
	UNITTEST_TIME_CONSTRAINT(50);

	CHECK(SDL_init_ok);
	if(SDL_init_ok)
	{
		vector<string> files;
		files.push_back("1");
		files.push_back("2");
		files.push_back("3");
		files.push_back("4");
		files.push_back("5");
		files.push_back("6");
		files.push_back("7");
		ItemBrowserWidget* ibw = new ItemBrowserWidget(files, Vector2i(3, 2), Vector2i(36, 27));
		ibw->OnItemClick.connect(ItemClickHandler);
		ibw->OnItemSelectedChanged.connect(ItemSelectionHandler);
		ibw->OnItemFocusedClick.connect(ItemFocusedClickHandler);
		//3x2 - 2 = 4 -> 1 full page, 1 page 3/4

		CHECK_EQUAL(6, ibw->GetChildren().size());
		CHECK_EQUAL(0, ibw->GetPage());

		Event e;
		e.event.mouse_event.x = 10;
		e.event.mouse_event.y = 10;
		e.event.mouse_event.btns = MouseButton::Left;
		e.event_type = EventType::MouseUp;

		ibw->HandleEvent(e);

		CHECK_EQUAL(1, item_click_count);
		CHECK_EQUAL("1", last_item_clicked);
		CHECK_EQUAL(1, item_selected_change_count);
		CHECK_EQUAL(0, item_focus_click_count);

		ibw->HandleEvent(e);
		CHECK_EQUAL(2, item_click_count);
		CHECK_EQUAL("1", last_item_clicked);
		CHECK_EQUAL(1, item_selected_change_count);
		CHECK_EQUAL(1, item_focus_click_count);

		e.event.mouse_event.x = 40;
		e.event.mouse_event.y = 10;
		ibw->HandleEvent(e);
		CHECK_EQUAL(3, item_click_count);
		CHECK_EQUAL("2", last_item_clicked);
		CHECK_EQUAL(2, item_selected_change_count);
		CHECK_EQUAL(1, item_focus_click_count);


		delete ibw;
	}
}
