#include "Logger.h"
#include "GameStateMachine.h"
#include <Widget.h>
#include <MenuWidget.h>
#include <GameGridWidget.h>
#include <ItemBrowserWidget.h>
#include <boost/lexical_cast.hpp>
#include "PuzzleLevel.h"
#include "Animation.h"
#include "SDLAnimationFrame.h"
#include "Settings.h"
#include <boost/foreach.hpp>
#include "Progress.h"
#include <boost/filesystem.hpp>

/* Consts */
const float GameStateMachine::sub_mode_widget_transition_time = 0.5f;

GameStateMachine::GameStateMachine()
{
	mode_timer_ = 0.01f;
	sub_mode_timer_ = 0;
	mode_ = Mode::Intro;
	pend_mode_ = Mode::Menu;
	sub_mode_ = Submode::None;
	pend_sub_mode_ = Submode::None;
	input_method_ = Settings::GetUseGestures() ? InputMethod::MouseDrags : InputMethod::MouseClicks;;
	options_widget_ = NULL;
	levels_widget_ = NULL;
	level_completed_ = false;
	page_of_level_ = 0;
	last_puzzle_mode_ = PuzzleMode::Puzzle;
	rel_path_ = "";
	LoadPuzzleFiles();
	puzzle_index_ = -1;
	SetupIntro();

	fade_timer_ = 0.0f;
	total_fade_time_ = 1.0f;
	
}

GameStateMachine::~GameStateMachine()
{
	Settings::SetUseGestures(input_method_ == InputMethod::MouseDrags);
}

/* Directory query functions */

void GameStateMachine::LoadPuzzleFiles()
{
	puzzle_files_.clear();
	if(rel_path_.length() > 1)
	{
		puzzle_files_.push_back("..");
	}
	if(!boost::filesystem::exists("Levels" + rel_path_))
		return;
	for(boost::filesystem::directory_iterator di("Levels" + rel_path_); di != boost::filesystem::directory_iterator(); ++di)
	{
		if(!boost::filesystem::is_directory(di->status()))
		{
			if(di->path().leaf() != "Folder.png")
			{
				puzzle_files_.push_back(di->path().leaf());
			}
		} else
		{
			puzzle_files_.push_back(di->path().leaf());
		}
	}
	std::sort(puzzle_files_.begin(), puzzle_files_.end());
}

/* Intro */

void GameStateMachine::SetupIntro()
{
}

void GameStateMachine::ProcessIntro(float _timespan)
{
}

void GameStateMachine::TeardownIntro()
{
	Widget::ClearRoot();
}

/* Menu */

void GameStateMachine::SetupMenu()
{
	reload_due_ = false;

	Widget* puzzle = new Widget("Blank128x32.png");
	puzzle->SetPosition(Vector2i(2, 2));
	puzzle->SetText("Puzzles", TextAlignment::Centre);
	Widget* editor = new Widget("Blank128x32.png");
	editor->SetPosition(Vector2i(2, 36));
	editor->SetText("Editor", TextAlignment::Centre);
	Widget* options = new Widget("Blank128x32.png");
	options->SetPosition(Vector2i(2, 70));
	options->SetText("Options", TextAlignment::Centre);
	Widget* exit = new Widget("Blank128x32.png");
	exit->SetPosition(Vector2i(2, 104));
	exit->SetText("Exit", TextAlignment::Centre);
	puzzle_index_ = -1;

	// Size = 72 * 5 = 360 x 54 * 5 = 270
	
	levels_widget_ = new ItemBrowserWidget(puzzle_files_, Vector2i(6, 6), Vector2i(78, 60));
	levels_widget_->SetPage(page_of_level_);
	levels_widget_->SetPosition(Vector2i(138, 0));
	levels_widget_->OnItemRender.connect(boost::bind(&GameStateMachine::RenderLevel, this, _1, _2, _3));
	levels_widget_->PerformItemLayout();
	levels_widget_->OnItemFocusedClick.connect(boost::bind(&GameStateMachine::MenuLevelSelect, this, _1, _2));
	levels_widget_->OnItemClick.connect(boost::bind(&GameStateMachine::MenuLevelHighlight, this, _1, _2));
	levels_widget_->OnPageChange.connect(boost::bind(&GameStateMachine::MenuLevelBrowerPageChange, this, _1, _2, _3));
	
	level_name_ = new Widget("Blank384x96.png");
	level_name_->SetPosition(Vector2i(138, 380));
	level_name_->SetText("Page " + boost::lexical_cast<std::string, int>(levels_widget_->GetPage() + 1) + "/" + boost::lexical_cast<std::string, int>(levels_widget_->GetPageCount()), TextAlignment::TopLeft);
	level_name_->SetRejectsFocus(true);


	options_widget_ = new Widget("Options.png");
	options_widget_->SetPosition(Vector2i(SDL_GetVideoSurface()->w, 0));
	options_widget_->SetRejectsFocus(true);

	Widget* input_method = new Widget("Blank384x32.png");
	input_method->OnClick.connect(boost::bind(&GameStateMachine::OptionsInputmethodCallback, this, _1));
	input_method->SetPosition(Vector2i(10, 10));
	options_widget_->AddChild(input_method);

	gesture_hint_ = new Widget("Gestures.png");
	gesture_hint_->SetPosition(Vector2i(10, 52));
	gesture_hint_->SetRejectsFocus(true);
	options_widget_->AddChild(gesture_hint_);
	clicks_hint_ = new Widget("Clicks.png");
	clicks_hint_->SetPosition(Vector2i(10, 52));
	clicks_hint_->SetRejectsFocus(true);
	options_widget_->AddChild(clicks_hint_);

	switch(input_method_)
	{
	case InputMethod::MouseClicks:
		input_method->SetText("Input: Clicks", TextAlignment::Centre);
		clicks_hint_->SetVisibility(true);
		gesture_hint_->SetVisibility(false);
		break;
	case InputMethod::MouseDrags:
		input_method->SetText("Input: Gestures", TextAlignment::Centre);
		clicks_hint_->SetVisibility(false);
		gesture_hint_->SetVisibility(true);
		break;
	}

	exit->OnClick.connect(boost::bind(&GameStateMachine::MenuExitCallback, this, _1));
	options->OnClick.connect(boost::bind(&GameStateMachine::MenuOptionsCallback, this, _1));
	puzzle->OnClick.connect(boost::bind(&GameStateMachine::MenuPuzzleCallback, this, _1));
	editor->OnClick.connect(boost::bind(&GameStateMachine::MenuEditorCallback, this, _1));

	sub_mode_ = Submode::None;
	pend_sub_mode_ = Submode::LevelList;
	sub_mode_timer_ = 1.0f;
}

void GameStateMachine::ProcessMenu(float _timespan)
{
	if(reload_due_)
	{
		levels_widget_->SetItems(puzzle_files_);
		reload_due_ = false;
	}


	Widget* active_sub_mode_widget = NULL;
	Widget* pending_sub_mode_widget = NULL;
	
	switch(sub_mode_)
	{
	case Submode::LevelList:
		active_sub_mode_widget = levels_widget_;
		break;
	case Submode::Options:
		active_sub_mode_widget = options_widget_;
		break;
	}
	switch(pend_sub_mode_)
	{
	case Submode::LevelList:
		pending_sub_mode_widget = levels_widget_;
		break;
	case Submode::Options:
		pending_sub_mode_widget = options_widget_;
		break;
	}
	if(sub_mode_ != pend_sub_mode_)
	{
		float out_fraction = 1.0f - (sub_mode_timer_ - sub_mode_widget_transition_time) / sub_mode_widget_transition_time;
		float in_fraction = sub_mode_timer_ / sub_mode_widget_transition_time;

		in_fraction = in_fraction < 0 ? 0: in_fraction > 1.0f ? 1.0f : in_fraction;
		out_fraction = out_fraction < 0 ? 0: out_fraction > 1.0f ? 1.0f : out_fraction;

		Vector2f in_position = Vector2f(138, 0) - Vector2f(0, SDL_GetVideoSurface()->h + 10) * in_fraction;
		Vector2f out_position = Vector2f(138, 0) + Vector2f(SDL_GetVideoSurface()->w, SDL_GetVideoSurface()->h + 10) * out_fraction ;

		if(active_sub_mode_widget)
			active_sub_mode_widget->SetPosition(out_position);
		if(pending_sub_mode_widget)
			pending_sub_mode_widget->SetPosition(in_position);
	} else
	{
		if(active_sub_mode_widget)
			active_sub_mode_widget->SetPosition(Vector2i(138, 10));
	}
}

void GameStateMachine::TeardownMenu()
{
	Widget::ClearRoot();
}

/* Menu events */

void GameStateMachine::RenderLevel(Widget* _widget, BlittableRect** _rect, std::string _name)
{
	if(boost::filesystem::is_directory("Levels" + rel_path_ + "/" + _name))
	{
		if(boost::filesystem::exists("Levels" + rel_path_ + "/" + _name + "/Folder.png"))
		{
			*_rect = new BlittableRect("Levels" + rel_path_ + "/" + _name + "/Folder.png", true);
		} else if(_name == "..")
		{
			*_rect = new BlittableRect("FolderUp.png");
		} else
		{
			*_rect = new BlittableRect("Folder.png");
		}
	} else
	{
		PuzzleLevel pl(rel_path_ + "/" + _name, Vector2f(6, 6));

		//72x54
		SDL_Surface* surface = SDL_CreateRGBSurface(SDL_GetVideoSurface()->flags, 6 * 13, 6 * 10, SDL_GetVideoSurface()->format->BytesPerPixel * 8, SDL_GetVideoSurface()->format->Rmask, SDL_GetVideoSurface()->format->Gmask, SDL_GetVideoSurface()->format->Bmask, SDL_GetVideoSurface()->format->Amask);
		SDL_Surface* conv_surface = SDL_DisplayFormatAlpha(surface);
		SDL_FreeSurface(surface);
		surface = conv_surface;
		*_rect = new BlittableRect(surface, false);
		(*_rect)->Fill(255, 255, 255, 255);
		//Rendering occurs to whatever SDLAnimationFrame::screen_ is set to, so to 
		//generate a mini preview it should be set to the preview surface
		SDLAnimationFrame::screen_ = surface;
		pl.grid_animation_->GetCurrentFrame()->Draw(Vector2f(3, 3));
		if(Progress::GetProgress(rel_path_ + "/" + _name).completed)
		{
			BlittableRect overlay("Tick.png");
			overlay.Blit(Vector2i(surface->w - overlay.GetSize().x - 4, surface->h - overlay.GetSize().y - 4), *_rect);
		}
	}
}

void GameStateMachine::MenuLevelSelect(Widget* _widget, std::string _name)
{
	if(pend_mode_ == Mode::Menu && mode_ == Mode::Menu)
	{
		if(boost::filesystem::is_directory("Levels" + rel_path_ + "/" + _name))
		{
			if(_name == "..")
			{
				boost::filesystem::path p = boost::filesystem::path(rel_path_);
				rel_path_ = p.parent_path().string();
				if(rel_path_ == "/")
					rel_path_ = "";
			} else
				rel_path_ = rel_path_ + "/" + _name ;
			
			LoadPuzzleFiles();
			reload_due_ = true;
		} else
		{
			puzzle_level_ = boost::shared_ptr<PuzzleLevel>(new PuzzleLevel(rel_path_ + "/" + _name, Settings::GetGridSize()));
			pend_mode_ = Mode::Puzzle;
			mode_timer_ = 1.0f;
			FadeInOut(2.0f);
			page_of_level_ = ((ItemBrowserWidget*)_widget)->GetPage();
			for(int i = 0; i < puzzle_files_.size(); i++)
			{
				if(_name.compare(puzzle_files_[i]) == 0)
				{
					puzzle_index_ = i;
				}
			}
		}
	}
}

void GameStateMachine::MenuLevelHighlight(Widget* _widget, std::string _name)
{
	Logger::DiagnosticOut() << _name << "\n";
	level_name_->SetText(_name, TextAlignment::TopLeft);
}

void GameStateMachine::MenuExitCallback(Widget* _widget)
{
	if(pend_mode_ == Mode::Menu &&
	   mode_ == Mode::Menu)
	{
		pend_mode_ = Mode::Exit;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::MenuPuzzleCallback(Widget *_widget)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_sub_mode_ = Submode::LevelList;
		sub_mode_timer_ = 1.0f;
	}
}

void GameStateMachine::MenuEditorCallback(Widget *_widget)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_mode_ = Mode::Editor;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::MenuOptionsCallback(Widget *_widget)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_sub_mode_ = Submode::Options;
		sub_mode_timer_ = 1.0f;
	}
}

void GameStateMachine::OptionsInputmethodCallback(Widget* _widget)
{
	switch(input_method_)
	{
	case InputMethod::MouseClicks:
		input_method_ = InputMethod::MouseDrags;
		_widget->SetText("Input: Gestures", TextAlignment::Centre);
		clicks_hint_->SetVisibility(false);
		gesture_hint_->SetVisibility(true);
		break;
	case InputMethod::MouseDrags:
		input_method_ = InputMethod::MouseClicks;
		_widget->SetText("Input: Clicks", TextAlignment::Centre);
		clicks_hint_->SetVisibility(true);
		gesture_hint_->SetVisibility(false);
		break;
	}
}

void GameStateMachine::MenuLevelBrowerPageChange(Widget* _widget, int _old_page, int _new_page)
{
	level_name_->SetText("Page " + boost::lexical_cast<std::string, int>(levels_widget_->GetPage() + 1) + "/" + boost::lexical_cast<std::string, int>(levels_widget_->GetPageCount()), TextAlignment::TopLeft);
}

/* Puzzle */

void GameStateMachine::SetupPuzzle()
{
	Widget* start = new Widget("Blank96x32.png");
	start->SetPosition(Vector2i(0,0));
	start->OnClick.connect(boost::bind(&GameStateMachine::PuzzleStartClick, this, _1));
	start->SetText("Start", TextAlignment::Centre);

	Widget* reset = new Widget("Blank96x32.png");
	reset->SetPosition(Vector2i(0, 32));
	reset->OnClick.connect(boost::bind(&GameStateMachine::PuzzleResetClick, this, _1));
	reset->SetText("Reset", TextAlignment::Centre);

	Widget* quit = new Widget("Blank96x32.png");
	quit->SetPosition(Vector2i(0,64));
	quit->OnClick.connect(boost::bind(&GameStateMachine::PuzzleQuitClick, this, _1));
	quit->SetText("Back", TextAlignment::Centre);

	GameGridWidget* grid_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32, 32));
	grid_widget->SetPosition(Vector2i(138, 10));
	grid_widget->OnGridClick.connect(boost::bind(&GameStateMachine::PuzzleGridClick, this, _1, _2));
	grid_widget->OnGridGesture.connect(boost::bind(&GameStateMachine::PuzzleGridGesture, this, _1, _2));

	puzzle_complete_widget_ = new Widget("Blank384x96.png");
	puzzle_complete_widget_->SetText("Solved!", TextAlignment::Top);
	puzzle_complete_widget_->SetRejectsFocus(true);
	Widget* next = new Widget("Blank96x32.png");
	next->SetText("Next", TextAlignment::Centre);
	next->SetPosition(Vector2i(72, 48));
	next->OnClick.connect(boost::bind(&GameStateMachine::PuzzleNextClick, this, _1));

	Widget* menu = new Widget("Blank96x32.png");
	menu->SetPosition(Vector2i(192, 48));
	menu->SetText("Back", TextAlignment::Centre);
	menu->OnClick.connect(boost::bind(&GameStateMachine::PuzzleQuitClick, this, _1));

	puzzle_complete_widget_->AddChild(next);
	puzzle_complete_widget_->AddChild(menu);
	puzzle_complete_widget_->SetPosition(Vector2i(0, - 300));
	//puzzle_complete_widget_->SetModal(false);


	level_completed_ = false;
}

void GameStateMachine::ProcessPuzzle(float _timespan)
{
	puzzle_level_->Tick(_timespan, input_);
	if(puzzle_level_->GetPuzzleState() == PuzzleMode::Puzzle && last_puzzle_mode_ == PuzzleMode::Victory)
	{
		puzzle_complete_widget_->SetPosition(Vector2i((SDL_GetVideoSurface()->w - 384) / 2, (SDL_GetVideoSurface()->h - 96) / 2));
		puzzle_complete_widget_->SetModal(true);
	}
	if(puzzle_level_->GetPuzzleState() == PuzzleMode::Victory && level_completed_ == false)
	{
		ProgressRecord pr;
		pr.completed = true;
		Progress::SetProgress(puzzle_level_->GetFilename(), pr);
		level_completed_ = true;
	}

	last_puzzle_mode_ = puzzle_level_->GetPuzzleState();
	input_ = Input();
}

void GameStateMachine::TeardownPuzzle()
{
	Widget::ClearRoot();
}

/* Puzzle events */

void GameStateMachine::PuzzleStartClick(Widget* _widget)
{
	input_.action = Action::Start;
}

void GameStateMachine::PuzzleResetClick(Widget* _widget)
{
	input_.action = Action::Cancel;
}

void GameStateMachine::PuzzleQuitClick(Widget* _widget)
{
	if(mode_ == Mode::Puzzle && pend_mode_ == Mode::Puzzle)
	{
		pend_mode_ = Mode::Menu;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::PuzzleNextClick(Widget* _widget)
{
	puzzle_index_++;
	puzzle_index_ %= puzzle_files_.size();
	puzzle_level_ = boost::shared_ptr<PuzzleLevel>(new PuzzleLevel(rel_path_ + "/" + puzzle_files_[puzzle_index_], Settings::GetGridSize()));
	puzzle_complete_widget_->SetModal(false);
	puzzle_complete_widget_->SetPosition(Vector2i(0, - 300));
	level_completed_ = false;
}

void GameStateMachine::PuzzleGridClick(Widget* _widget, MouseEventArgs _args)
{
	if(input_method_ == InputMethod::MouseClicks)
	{
		switch(_args.btns)
		{
		case MouseButton::Left:
			input_.action = Action::PlaceWestArrow;
			break;
		case MouseButton::Right:
			input_.action = Action::PlaceEastArrow;
			break;
		case MouseButton::ScrollUp:
			input_.action = Action::PlaceNorthArrow;
			break;
		case MouseButton::ScrollDown:
			input_.action = Action::PlaceSouthArrow;
			break;
		case MouseButton::Middle:
			input_.action = Action::Cancel;
			break;
		}
	} else
	{
		switch(_args.btns)
		{
		case MouseButton::Left:
			input_.action = Action::ClearSquare;
			break;
		case MouseButton::Middle:
			input_.action = Action::Cancel;
			break;
		case MouseButton::Right:
			input_.action = Action::ClearSquare;
			break;
		}
	}
	input_.position = Vector2i(_args.x, _args.y);
}

void GameStateMachine::PuzzleGridGesture(Widget* _widget, GridGestureEventArgs _args)
{
	if(input_method_ != InputMethod::MouseDrags)
		return;
	switch(_args.direction)
	{
	case GestureDirection::North:
		input_.action = Action::PlaceNorthArrow;
		break;
	case GestureDirection::South:
		input_.action = Action::PlaceSouthArrow;
		break;
	case GestureDirection::East:
		input_.action = Action::PlaceEastArrow;
		break;
	case GestureDirection::West:
		input_.action = Action::PlaceWestArrow;
		break;
	case GestureDirection::Center:
		input_.action = Action::ClearSquare;
	}
	input_.position = Vector2i(_args.x, _args.y);
	Logger::DiagnosticOut() << "Gesture " << input_.action << "\n";
}

void GameStateMachine::PuzzleScrollDownClick(Widget* _widget)
{
	input_.action = Action::ScrollSouth;
}

void GameStateMachine::PuzzleScrollUpClick(Widget* _widget)
{
	input_.action = Action::ScrollNorth;
}

void GameStateMachine::PuzzleScrollLeftClick(Widget* _widget)
{
	input_.action = Action::ScrollWest;
}

void GameStateMachine::PuzzleScrollRightClick(Widget* _widget)
{
	input_.action = Action::ScrollEast;
}

/* Editor methods */
void GameStateMachine::SetupEditor()
{
	Widget* scroll_down = new Widget("ScrollDown.png");
	scroll_down->SetPosition(Vector2i(138, 9 * Settings::GetGridSize().y + 11));
	scroll_down->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollDownClick, this, _1));

	Widget* scroll_up = new Widget("ScrollUp.png");
	scroll_up->SetPosition(Vector2i(138, 0));
	scroll_up->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollUpClick, this, _1));

	Widget* scroll_left = new Widget("ScrollLeft.png");
	scroll_left->SetPosition(Vector2i(128, 10));
	scroll_left->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollLeftClick, this, _1));

	Widget* scroll_right = new Widget("ScrollRight.png");
	scroll_right->SetPosition(Vector2i(12 * Settings::GetGridSize().x + 138 + 1, 10));
	scroll_right->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollRightClick, this, _1));

	GameGridWidget* grid_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32, 32));
	grid_widget->SetPosition(Vector2i(138, 10));
	grid_widget->OnGridClick.connect(boost::bind(&GameStateMachine::PuzzleGridClick, this, _1, _2));
	grid_widget->OnGridGesture.connect(boost::bind(&GameStateMachine::PuzzleGridGesture, this, _1, _2));


	size_ = Vector2i(12, 9);
	new_level_widget_ = new Widget("Blank384x384.png");
	new_level_widget_->SetPosition(Vector2i(128, 48));
	new_level_widget_->SetRejectsFocus(true);
	new_level_widget_->SetVisibility(true);
	new_level_widget_->SetModal(true);

	Widget* sizex_desc = new Widget("Blank32x32.png");
	sizex_desc->SetPosition(Vector2i(10, 10));
	sizex_desc->SetText("x=", TextAlignment::Centre);
	sizex_desc->SetRejectsFocus(true);

	Widget* sizeppx = new Widget("Blank32x32.png");
	sizeppx->SetText("+", TextAlignment::Centre);
	sizeppx->SetPosition(Vector2i(200, 10));
	sizeppx->OnClick.connect(boost::bind(&GameStateMachine::EditorSizePPXClick, this, _1));

	Widget* sizemmx = new Widget("Blank32x32.png");
	sizemmx->SetText("-", TextAlignment::Centre);
	sizemmx->SetPosition(Vector2i(52, 10));
	sizemmx->OnClick.connect(boost::bind(&GameStateMachine::EditorSizeMMXClick, this, _1));

	sizex = new Widget("Blank96x32.png");
	sizex->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
	sizex->SetPosition(Vector2i(94, 10));
	sizex->SetRejectsFocus(true);

	Widget* sizey_desc = new Widget("Blank32x32.png");
	sizey_desc->SetPosition(Vector2i(10, 52));
	sizey_desc->SetText("y=", TextAlignment::Centre);
	sizey_desc->SetRejectsFocus(true);

	Widget* sizeppy = new Widget("Blank32x32.png");
	sizeppy->SetText("+", TextAlignment::Centre);
	sizeppy->SetPosition(Vector2i(200, 52));
	sizeppy->OnClick.connect(boost::bind(&GameStateMachine::EditorSizePPYClick, this, _1));

	Widget* sizemmy = new Widget("Blank32x32.png");
	sizemmy->SetText("-", TextAlignment::Centre);
	sizemmy->SetPosition(Vector2i(52, 52));
	sizemmy->OnClick.connect(boost::bind(&GameStateMachine::EditorSizeMMYClick, this, _1));

	sizey = new Widget("Blank96x32.png");
	sizey->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
	sizey->SetPosition(Vector2i(94, 52));
	sizey->SetRejectsFocus(true);

	Widget* level_name = new Widget("Blank364x32.png");
	level_name->SetText("Unnamed", TextAlignment::Centre);
	level_name->SetPosition(Vector2i(10, 94));
	level_name->SetEditable(true);

	Widget* create_level = new Widget("Blank128x32.png");
	create_level->SetText("Create!", TextAlignment::Centre);
	create_level->SetPosition(Vector2i(10, 136));
	create_level->OnClick.connect(boost::bind(&GameStateMachine::EditorCreateClick, this, _1));



	new_level_widget_->AddChild(sizex_desc);
	new_level_widget_->AddChild(sizeppx);
	new_level_widget_->AddChild(sizemmx);
	new_level_widget_->AddChild(sizex);

	new_level_widget_->AddChild(sizey_desc);
	new_level_widget_->AddChild(sizeppy);
	new_level_widget_->AddChild(sizemmy);
	new_level_widget_->AddChild(sizey);

	new_level_widget_->AddChild(level_name);
	new_level_widget_->AddChild(create_level);


	



	Widget* makenew = new Widget("Blank128x32.png");
	makenew->SetPosition(Vector2i(0, 2));
	makenew->SetText("New", TextAlignment::Centre);
	makenew->OnClick.connect(boost::bind(&GameStateMachine::EditorNewClick, this, _1));

	Widget* wall_mode = new Widget("Blank96x32.png");
	wall_mode->SetPosition(Vector2i(0, 36));
	wall_mode->SetText("Walls", TextAlignment::Centre);
	wall_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorWallMode, this, _1));

	Widget* mouse_mode = new Widget("EditorMice.png");
	mouse_mode->SetPosition(Vector2i(0, 70));
	mouse_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorMouseMode, this, _1));

	Widget* cat_mode = new Widget("EditorCats.png");
	cat_mode->SetPosition(Vector2i(0, 104));
	cat_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorCatMode, this, _1));

	Widget* hole_mode = new Widget("Blank96x32.png");
	hole_mode->SetPosition(Vector2i(0, 138));
	hole_mode->SetText("Holes", TextAlignment::Centre);
	hole_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorHoleMode, this, _1));

	Widget* rocket_mode = new Widget("Blank96x32.png");
	rocket_mode->SetPosition(Vector2i(0, 172));
	rocket_mode->SetText("Rocket", TextAlignment::Centre);
	rocket_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorRocketMode, this, _1));

	Widget* arrow_mode = new Widget("Blank96x32.png");
	arrow_mode->SetPosition(Vector2i(0, 206));
	arrow_mode->SetText("Arrow", TextAlignment::Centre);
	arrow_mode->OnClick.connect(boost::bind(&GameStateMachine::EditorArrowMode, this, _1));

	Widget* start = new Widget("Blank128x32.png");
	start->SetPosition(Vector2i(0, 240));
	start->SetText("Start", TextAlignment::Centre);
	start->OnClick.connect(boost::bind(&GameStateMachine::EditorStartClick, this, _1));

	Widget* reset = new Widget("Blank128x32.png");
	reset->SetPosition(Vector2i(0, 274));
	reset->SetText("Reset", TextAlignment::Centre);
	reset->OnClick.connect(boost::bind(&GameStateMachine::EditorResetClick, this, _1));

	Widget* save = new Widget("Blank128x32.png");
	save->SetPosition(Vector2i(0, 308));
	save->SetText("Save", TextAlignment::Centre);
	save->OnClick.connect(boost::bind(&GameStateMachine::EditorSaveClick, this, _1));

	Widget* back = new Widget("Blank128x32.png");
	back->SetPosition(Vector2i(0, 342));
	back->SetText("Return", TextAlignment::Centre);
	back->OnClick.connect(boost::bind(&GameStateMachine::EditorReturnClick, this, _1));

	

	editor_level_ = boost::shared_ptr<EditLevel>((EditLevel*)NULL);	
}

void GameStateMachine::TeardownEditor()
{
	Widget::ClearRoot();
}

void GameStateMachine::ProcessEditor(float _timespan)
{
	if(editor_level_.get() != NULL)
		editor_level_->Tick(_timespan, input_);
	input_ = Input();
}

/* Editor event handling */

void GameStateMachine::EditorReturnClick(Widget *_widget)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_mode_ = Mode::Menu;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::EditorSizePPXClick(Widget* _widget)
{
	size_.x++;
	size_.x = size_.x < 3 ? 3 : size_.x > 50 ? 50 : size_.x;
	sizex->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
}

void GameStateMachine::EditorSizeMMXClick(Widget* _widget)
{
	size_.x--;
	size_.x = size_.x < 3 ? 3 : size_.x > 50 ? 50 : size_.x;
	sizex->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
}

void GameStateMachine::EditorSizePPYClick(Widget* _widget)
{
	size_.y++;
	size_.y = size_.y < 3 ? 3 : size_.y > 50 ? 50 : size_.y;
	sizey->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
}

void GameStateMachine::EditorSizeMMYClick(Widget* _widget)
{
	size_.y--;
	size_.y = size_.y < 3 ? 3 : size_.y > 50 ? 50 : size_.y;
	sizey->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
}

void GameStateMachine::EditorCreateClick(Widget* _widget)
{
	editor_level_ = boost::shared_ptr<EditLevel>(new EditLevel(size_, Settings::GetGridSize()));
	new_level_widget_->SetVisibility(false);
	new_level_widget_->SetModal(false);
	new_level_widget_->SetPosition(Vector2i(-500, -500));
}

void GameStateMachine::EditorWallMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditWalls);
}

void GameStateMachine::EditorMouseMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditMice);
}

void GameStateMachine::EditorCatMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditCats);
}

void GameStateMachine::EditorHoleMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditHoles);
}

void GameStateMachine::EditorRocketMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditRockets);
}

void GameStateMachine::EditorArrowMode(Widget* _widget)
{
	editor_level_->SetEditMode(EditMode::EditArrows);
}

void GameStateMachine::EditorNewClick(Widget* _widget)
{
	new_level_widget_->SetVisibility(true);
	new_level_widget_->SetModal(true);
	new_level_widget_->SetPosition(Vector2i(128, 48));
}

void GameStateMachine::EditorStartClick(Widget* _widget)
{
	input_.action = Action::Start;
}

void GameStateMachine::EditorResetClick(Widget* _widget)
{
	input_.action = Action::Cancel;
}

void GameStateMachine::EditorSaveClick(Widget* _widget)
{
	editor_level_->Save("Editor.Level");
}

/* Main methods */

bool GameStateMachine::Tick(float _timespan)
{
	mode_timer_ -= _timespan;
	sub_mode_timer_ -= _timespan;
	fade_timer_ -= _timespan;
	//Mode transitions
	if(mode_ != pend_mode_ && mode_timer_ < 0)
	{
		sub_mode_ = Submode::None;
		switch(mode_)
		{
		case Mode::Intro:
			TeardownIntro();
			break;
		case Mode::Menu:
			TeardownMenu();
			break;
		case Mode::Editor:
			TeardownEditor();
			break;
		case Mode::Puzzle:
			TeardownPuzzle();
			break;
		}		

		switch(pend_mode_)
		{
		case Mode::Intro:
			SetupIntro();
			break;
		case Mode::Menu:
			SetupMenu();
			break;
		case Mode::Puzzle:
			SetupPuzzle();
			break;
		case Mode::Editor:
			SetupEditor();
			break;
		case Mode::Exit:
			break;
		}

		mode_ = pend_mode_;
	}

	if(sub_mode_ != pend_sub_mode_ && sub_mode_timer_ < 0)
	{
		sub_mode_ = pend_sub_mode_;
	}

	switch(mode_)
	{
	case Mode::Intro:
		ProcessIntro(_timespan);
		break;
	case Mode::Menu:
		ProcessMenu(_timespan);
		break;
	case Mode::Puzzle:
		ProcessPuzzle(_timespan);
		break;
	case Mode::Editor:
		ProcessEditor(_timespan);
		break;
	case Mode::Exit:
		return true;
		break;
	}
	return false;
}

void GameStateMachine::Draw(SDL_Surface* _target)
{
	/* /-\ */
	float fade = 0;
	if(fade_timer_ > total_fade_time_ * 0.6f)
		fade = 1.0f - ((fade_timer_ - total_fade_time_ * 0.6f) / (total_fade_time_ * 0.4f));
	else if (fade_timer_ > total_fade_time_ * 0.4f)
		fade = 1.0f;
	else if(fade_timer_ > 0)
		fade = (fade_timer_ / (total_fade_time_ * 0.4f));
	else
		fade = 0;
	Widget::SetFade(fade);

	std::vector<RenderItem> render_items;
	switch(mode_)
	{
	case Mode::Editor:
		if(editor_level_.get() != NULL)
		{
			render_items = editor_level_->Draw();
		}
		break;
	case Mode::Puzzle:
		if(puzzle_level_.get() != NULL)
		{
			render_items = puzzle_level_->Draw();
		}
		break;
	}

	//Sort front to back to prevent overlay issues
	std::sort(render_items.begin(), render_items.end(), RenderItem::DepthSort<RenderItem>());
	SDLAnimationFrame::screen_ = _target;
	int frame = 0;


	BOOST_FOREACH(RenderItem& ri, render_items)
	{
		ri.frame_->Draw(ri.position_);
		if(ri.position_.x < 0)
			ri.frame_->Draw(ri.position_ + Vector2f(Settings::GetGridSize().x * size_.x, 0));
		if(ri.position_.x > Settings::GetGridSize().x * size_.x - Settings::GetGridSize().x)
			ri.frame_->Draw(Vector2f(ri.position_.x - Settings::GetGridSize().x * size_.x, ri.position_.y));
		if(ri.position_.y < 0)
			ri.frame_->Draw(ri.position_ + Vector2f(0, Settings::GetGridSize().y * size_.y));
		if(ri.position_.y > Settings::GetGridSize().y * size_.y - Settings::GetGridSize().y)
			ri.frame_->Draw(Vector2f(ri.position_.x, ri.position_.y - Settings::GetGridSize().y * size_.y));
	}
}

void GameStateMachine::FadeInOut(float _total_time)
{
	total_fade_time_ = _total_time;
	fade_timer_ = _total_time;
}