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
#include <boost/tokenizer.hpp>


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
	render_area_ = NULL;
	scroll_left_widget_ = NULL;
	scroll_right_widget_ = NULL;
	scroll_up_widget_ = NULL;
	scroll_down_widget_ = NULL;

	Widget::OnGlobalKeyUp.connect(boost::bind(&GameStateMachine::KeyboardCallback, this, _1, _2));
	state_indicator_level_ = boost::shared_ptr<StatusLevel>(new StatusLevel(Settings::GetGridSize()));
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
			//This would be better if it checked for levels instead
			if(di->path().leaf() != "Folder.Icon")
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

void GameStateMachine::ProcessIntro(float /*_timespan*/)
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
	Widget* multiplayer = new Widget("Blank128x64.png");
	multiplayer->SetPosition(Vector2i(2, 104));
	multiplayer->SetText("Play\nOnline", TextAlignment::Centre);

	Widget* exit = new Widget("Blank128x32.png");
	exit->SetPosition(Vector2i(2, 170));
	exit->SetText("Exit", TextAlignment::Centre);
	puzzle_index_ = -1;
	
	levels_widget_ = new ItemBrowserWidget(puzzle_files_, Vector2i((Settings::GetResolution().x - 172) / 78, (Settings::GetResolution().y - 120) / 60), Vector2i(78, 60));
	levels_widget_->SetPage(page_of_level_);
	levels_widget_->SetPosition(Vector2i(138, 0));
	levels_widget_->OnItemRender.connect(boost::bind(&GameStateMachine::RenderLevel, this, _1, _2, _3));
	levels_widget_->PerformItemLayout();
	levels_widget_->OnItemFocusedClick.connect(boost::bind(&GameStateMachine::MenuLevelSelect, this, _1, _2));
	levels_widget_->OnItemSelectedChanged.connect(boost::bind(&GameStateMachine::MenuLevelHighlight, this, _1, _2));
	levels_widget_->OnPageChange.connect(boost::bind(&GameStateMachine::MenuLevelBrowerPageChange, this, _1, _2, _3));
	
	level_name_ = new Widget("Blank384x96.png");
	level_name_->SetPosition(Vector2i(138, Settings::GetResolution().y - 120 + 20));
	level_name_->SetText("Page " + boost::lexical_cast<std::string, int>(levels_widget_->GetPage() + 1) + "/" + boost::lexical_cast<std::string, int>(levels_widget_->GetPageCount()), TextAlignment::TopLeft);
	level_name_->SetRejectsFocus(true);

	edit_widget_ = new Widget("Blank96x32.png");
	edit_widget_->SetPosition(Vector2i(138+384+10, Settings::GetResolution().y - 120 + 20));
	edit_widget_->SetText("Edit", TextAlignment::Centre);
	edit_widget_->SetVisibility(false);
	edit_widget_->OnClick.connect(boost::bind(&GameStateMachine::MenuEditExistingCallback, this, _1));



	options_widget_ = new Widget("Options.png");
	options_widget_->SetPosition(Vector2i(SDL_GetVideoSurface()->w, 0));
	options_widget_->SetRejectsFocus(true);

	Widget* input_method = new Widget("Blank384x32.png");
	input_method->OnClick.connect(boost::bind(&GameStateMachine::OptionsInputmethodCallback, this, _1));
	input_method->SetPosition(Vector2i(10, 10));
	options_widget_->AddChild(input_method);
	options_widget_->SetText(" WASD keys place arrows\n Arrow keys scroll\n Escape resets\n Space starts", TextAlignment::BottomLeft);

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
	multiplayer->OnClick.connect(boost::bind(&GameStateMachine::MenuPlayOnlineCallback, this, _1));
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
	state_indicator_level_->Tick(_timespan, input_);


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

		Vector2f in_position = Vector2f(138, 0) - Vector2f(0, static_cast<float>(SDL_GetVideoSurface()->h) + 10) * in_fraction;
		Vector2f out_position = Vector2f(138, 0) + Vector2f(static_cast<float>(SDL_GetVideoSurface()->w), static_cast<float>(SDL_GetVideoSurface()->h) + 10) * out_fraction ;

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

void GameStateMachine::RenderLevel(Widget* /*_widget*/, BlittableRect** _rect, std::string _name)
{
	if(boost::filesystem::is_directory("Levels" + rel_path_ + "/" + _name))
	{
		if(boost::filesystem::exists("Levels" + rel_path_ + "/" + _name + "/Folder.icon"))
		{
			*_rect = new BlittableRect("Levels" + rel_path_ + "/" + _name + "/Folder.icon", true);
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

		//72x60
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
			for(unsigned int i = 0; i < puzzle_files_.size(); i++)
			{
				if(_name.compare(puzzle_files_[i]) == 0)
				{
					puzzle_index_ = i;
				}
			}
		}
	}
}

void GameStateMachine::MenuLevelHighlight(Widget* /*_widget*/, std::string _name)
{
	Logger::DiagnosticOut() << _name << "\n";
	level_name_->SetText(_name, TextAlignment::TopLeft);
	if(boost::filesystem::is_directory("Levels" + rel_path_ + "/" + _name))
	{
		selected_level_ = "";
		edit_widget_->SetVisibility(false);
	} else
	{
		selected_level_ = rel_path_ + "/" + _name;
		edit_widget_->SetVisibility(true);
	}
}

void GameStateMachine::MenuExitCallback(Widget* /*_widget*/)
{
	if(pend_mode_ == Mode::Menu &&
	   mode_ == Mode::Menu)
	{
		pend_mode_ = Mode::Exit;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::MenuPuzzleCallback(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_sub_mode_ = Submode::LevelList;
		sub_mode_timer_ = 1.0f;
	}
}

void GameStateMachine::MenuEditorCallback(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_mode_ = Mode::Editor;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
		create_new_level_ = true;
	}
}

void GameStateMachine::MenuEditExistingCallback(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_ && selected_level_.length() > 0)
	{
		pend_mode_ = Mode::Editor;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
		create_new_level_ = false;
	}
}
void GameStateMachine::MenuOptionsCallback(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_sub_mode_ = Submode::Options;
		sub_mode_timer_ = 1.0f;
	}
}

void GameStateMachine::MenuPlayOnlineCallback(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_mode_ = Mode::ServerBrowser;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
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

void GameStateMachine::MenuLevelBrowerPageChange(Widget* /*_widget*/, int /*_old_page*/, int /*_new_page*/)
{
	level_name_->SetText("Page " + boost::lexical_cast<std::string, int>(levels_widget_->GetPage() + 1) + "/" + boost::lexical_cast<std::string, int>(levels_widget_->GetPageCount()), TextAlignment::TopLeft);
}

/* Puzzle */

void GameStateMachine::SetupPuzzle()
{
	CreateRenderArea(puzzle_level_->GetLevelSize(), Mode::Puzzle);

	arrow_stock_widget_ = new Widget("ArrowsArea.png");
	arrow_stock_widget_->SetPosition(Vector2i(126, puzzle_level_->GetLevelSize().y * Settings::GetGridSize().y + 30));
	arrow_stock_widget_->SetRejectsFocus(true);
	arrow_hash_ = 0;

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

//	GameGridWidget* grid_widget = new GameGridWidget(Vector2i(12, 9), Vector2i(32, 32));
//	grid_widget->SetPosition(Vector2i(138, 10));
//	grid_widget->OnGridClick.connect(boost::bind(&GameStateMachine::PuzzleGridClick, this, _1, _2));
//	grid_widget->OnGridGesture.connect(boost::bind(&GameStateMachine::PuzzleGridGesture, this, _1, _2));

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
		state_indicator_level_->SetState(StatusState::Success);
	}
	if(arrow_stock_widget_ && puzzle_level_ && arrow_hash_ != puzzle_level_->ComputeArrowHash())
	{
		LayoutArrows(puzzle_level_->GetArrows());
		arrow_hash_ = puzzle_level_->ComputeArrowHash();
	}
	switch(puzzle_level_->GetPuzzleState())
	{
	case PuzzleMode::Puzzle:
		state_indicator_level_->SetState(StatusState::Stopped);
		break;
	case PuzzleMode::Running:
		state_indicator_level_->SetState(StatusState::Running);
		break;
	case PuzzleMode::RunningFast:
		state_indicator_level_->SetState(StatusState::RunningFast);
		break;
	case PuzzleMode::Victory:
		state_indicator_level_->SetState(StatusState::Success);
		break;
	case PuzzleMode::Defeat:
		state_indicator_level_->SetState(StatusState::Fail);
		break;
	}
	state_indicator_level_->Tick(_timespan, Input());
	

	last_puzzle_mode_ = puzzle_level_->GetPuzzleState();
	input_ = Input();
}

void GameStateMachine::TeardownPuzzle()
{
	Widget::ClearRoot();
	arrow_stock_widget_ = NULL;
	//This prevents double freeing
	scroll_left_widget_ = NULL;
	scroll_right_widget_ = NULL;
	scroll_up_widget_ = NULL;
	scroll_down_widget_ = NULL;
}

/* Puzzle events */

void GameStateMachine::PuzzleStartClick(Widget* /*_widget*/)
{
	input_.action = Action::Start;
}

void GameStateMachine::PuzzleResetClick(Widget* /*_widget*/)
{
	input_.action = Action::Cancel;
}

void GameStateMachine::PuzzleQuitClick(Widget* /*_widget*/)
{
	if(mode_ == Mode::Puzzle && pend_mode_ == Mode::Puzzle)
	{
		pend_mode_ = Mode::Menu;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::PuzzleNextClick(Widget* /*_widget*/)
{
	puzzle_index_++;
	puzzle_index_ %= puzzle_files_.size();
	puzzle_level_ = boost::shared_ptr<PuzzleLevel>(new PuzzleLevel(rel_path_ + "/" + puzzle_files_[puzzle_index_], Settings::GetGridSize()));
	CreateRenderArea(puzzle_level_->GetLevelSize(), Mode::Puzzle);
	arrow_stock_widget_->SetPosition(Vector2i(106, puzzle_level_->GetLevelSize().y * Settings::GetGridSize().y + 20));
	arrow_hash_ = 0;
	puzzle_complete_widget_->SetModal(false);
	puzzle_complete_widget_->SetPosition(Vector2i(0, - 300));
	level_completed_ = false;
}

void GameStateMachine::PuzzleGridClick(Widget* /*_widget*/, MouseEventArgs _args)
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

void GameStateMachine::PuzzleGridGesture(Widget* /*_widget*/, GridGestureEventArgs _args)
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

void GameStateMachine::PuzzleScrollDownClick(Widget* /*_widget*/)
{
	input_.action = Action::ScrollSouth;
}

void GameStateMachine::PuzzleScrollUpClick(Widget* /*_widget*/)
{
	input_.action = Action::ScrollNorth;
}

void GameStateMachine::PuzzleScrollLeftClick(Widget* /*_widget*/)
{
	input_.action = Action::ScrollWest;
}

void GameStateMachine::PuzzleScrollRightClick(Widget* /*_widget*/)
{
	input_.action = Action::ScrollEast;
}

/* Editor methods */
void GameStateMachine::SetupEditor()
{
	size_ = Vector2i(12, 9);
	new_level_widget_ = new Widget("Blank384x384.png");
	new_level_widget_->SetRejectsFocus(true);

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

	sizex_ = new Widget("Blank96x32.png");
	sizex_->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
	sizex_->SetPosition(Vector2i(94, 10));
	sizex_->SetRejectsFocus(true);

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

	sizey_ = new Widget("Blank96x32.png");
	sizey_->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
	sizey_->SetPosition(Vector2i(94, 52));
	sizey_->SetRejectsFocus(true);

	level_name_editor_ = new Widget("Blank364x32.png");
	level_name_editor_->SetText("Unnamed", TextAlignment::Centre);
	level_name_editor_->SetPosition(Vector2i(10, 94));
	level_name_editor_->SetEditable(true);

	Widget* create_level = new Widget("Blank128x32.png");
	create_level->SetText("Create!", TextAlignment::Centre);
	create_level->SetPosition(Vector2i(10, 136));
	create_level->OnClick.connect(boost::bind(&GameStateMachine::EditorCreateClick, this, _1));



	new_level_widget_->AddChild(sizex_desc);
	new_level_widget_->AddChild(sizeppx);
	new_level_widget_->AddChild(sizemmx);
	new_level_widget_->AddChild(sizex_);

	new_level_widget_->AddChild(sizey_desc);
	new_level_widget_->AddChild(sizeppy);
	new_level_widget_->AddChild(sizemmy);
	new_level_widget_->AddChild(sizey_);

	new_level_widget_->AddChild(level_name_editor_);
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

	
	if(create_new_level_)
	{
		editor_level_ = boost::shared_ptr<EditLevel>((EditLevel*)NULL);	
		new_level_widget_->SetVisibility(true);
		new_level_widget_->SetModal(true);
		new_level_widget_->SetPosition(Vector2i(128, 48));
	} else
	{
		editor_level_ = boost::shared_ptr<EditLevel>(new EditLevel(selected_level_, Settings::GetGridSize()));	
		size_ = editor_level_->GetSize();
		new_level_widget_->SetVisibility(false);
		new_level_widget_->SetModal(false);
		new_level_widget_->SetPosition(Vector2i(-500, -500));
		CreateRenderArea(editor_level_->GetLevelSize(), Mode::Editor);
	}

}

void GameStateMachine::TeardownEditor()
{
	Widget::ClearRoot();
	//This prevents double freeing
	scroll_left_widget_ = NULL;
	scroll_right_widget_ = NULL;
	scroll_up_widget_ = NULL;
	scroll_down_widget_ = NULL;
}

void GameStateMachine::ProcessEditor(float _timespan)
{
	if(editor_level_.get() != NULL)
		editor_level_->Tick(_timespan, input_);
	input_ = Input();
}

/* Editor event handling */

void GameStateMachine::EditorReturnClick(Widget* /*_widget*/)
{
	if(sub_mode_ == pend_sub_mode_)
	{
		pend_mode_ = Mode::Menu;
		mode_timer_ = 1.0f;
		FadeInOut(2.0f);
	}
}

void GameStateMachine::EditorSizePPXClick(Widget* /*_widget*/)
{
	size_.x++;
	size_.x = size_.x < 3 ? 3 : size_.x > 50 ? 50 : size_.x;
	sizex_->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
}

void GameStateMachine::EditorSizeMMXClick(Widget* /*_widget*/)
{
	size_.x--;
	size_.x = size_.x < 3 ? 3 : size_.x > 50 ? 50 : size_.x;
	sizex_->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
}

void GameStateMachine::EditorSizePPYClick(Widget* /*_widget*/)
{
	size_.y++;
	size_.y = size_.y < 3 ? 3 : size_.y > 50 ? 50 : size_.y;
	sizey_->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
}

void GameStateMachine::EditorSizeMMYClick(Widget* /*_widget*/)
{
	size_.y--;
	size_.y = size_.y < 3 ? 3 : size_.y > 50 ? 50 : size_.y;
	sizey_->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);
}

void GameStateMachine::EditorCreateClick(Widget* /*_widget*/)
{
	editor_level_ = boost::shared_ptr<EditLevel>(new EditLevel(size_, Settings::GetGridSize()));
	CreateRenderArea(editor_level_->GetLevelSize(), Mode::Editor);
	new_level_widget_->SetVisibility(false);
	new_level_widget_->SetModal(false);
	new_level_widget_->SetPosition(Vector2i(-500, -500));
	editor_level_->SetName(level_name_editor_->GetText());
	selected_level_ = "";
}

void GameStateMachine::EditorWallMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditWalls);
}

void GameStateMachine::EditorMouseMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditMice);
}

void GameStateMachine::EditorCatMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditCats);
}

void GameStateMachine::EditorHoleMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditHoles);
}

void GameStateMachine::EditorRocketMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditRockets);
}

void GameStateMachine::EditorArrowMode(Widget* /*_widget*/)
{
	editor_level_->SetEditMode(EditMode::EditArrows);
}

void GameStateMachine::EditorNewClick(Widget* /*_widget*/)
{
	new_level_widget_->SetVisibility(true);
	new_level_widget_->SetModal(true);
	new_level_widget_->SetPosition(Vector2i(128, 48));
	size_ = Vector2i(12, 9);
	sizex_->SetText(boost::lexical_cast<std::string, int>(size_.x), TextAlignment::Centre);
	sizey_->SetText(boost::lexical_cast<std::string, int>(size_.y), TextAlignment::Centre);

}

void GameStateMachine::EditorStartClick(Widget* /*_widget*/)
{
	input_.action = Action::Start;
}

void GameStateMachine::EditorResetClick(Widget* /*_widget*/)
{
	input_.action = Action::Cancel;
}

void GameStateMachine::EditorSaveClick(Widget* /*_widget*/)
{
	if(selected_level_.length() > 0)
		editor_level_->Save(selected_level_);
	else
		editor_level_->Save(std::string("Editor - ") + editor_level_->GetName() + ".Level");
}

/* Server Browser */
void GameStateMachine::SetupServerBrowser()
{
	Widget* join = new Widget("Blank128x64.png");
	join->SetPosition(Vector2i(2,2));
	join->SetText("Join", TextAlignment::Centre);

	Widget* back = new Widget("Blank128x64.png");
	back->SetPosition(Vector2i(2, SDL_GetVideoSurface()->h - 66));
	back->SetText("Back", TextAlignment::Centre);
	back->OnClick.connect(boost::bind(&GameStateMachine::ServerBrowserBackCallback, this, _1));

	Widget* entry_bar = new Widget("IPBar.png");
	entry_bar->SetPosition(Vector2i(132, 2));
	entry_bar->SetRejectsFocus(true);

	Widget* ip_area = new Widget("IPArea.png");
	ip_area->SetPosition(Vector2i(8, 8));
	ip_area->SetText("127.0.0.1", TextAlignment::Left);
	ip_area->SetEditable(true);
	entry_bar->AddChild(ip_area);
	

	Widget* port_area = new Widget("PortArea.png");
	port_area->SetPosition(Vector2i(336, 8));
	port_area->SetText("9020", TextAlignment::Left);
	port_area->SetEditable(true);
	entry_bar->AddChild(port_area);

	std::vector<std::string> server_list;
	server_list.push_back("127.0.0.1:9020");
	server_list.push_back("localhost:9020");
	
	
	ItemBrowserWidget* servers = new ItemBrowserWidget(server_list, Vector2i(SDL_GetVideoSurface()->w-132, SDL_GetVideoSurface()->h-132) / Vector2i(78, 60), Vector2i(78, 60));
	servers->SetPosition(Vector2i(132, 68));
	servers->OnItemRender.connect(boost::bind(&GameStateMachine::ServerBrowserListRender, this, _1, _2, _3));
	servers->PerformItemLayout();
	
}

void GameStateMachine::ProcessServerBrowser(float _timespan)
{
}

void GameStateMachine::TeardownServerBrowser()
{
	Widget::ClearRoot();
}
/* Server Browser event handling */
void GameStateMachine::ServerBrowserBackCallback(Widget* _widget)
{
	pend_mode_ = Mode::Menu;
	mode_timer_ = 1.0f;
	FadeInOut(2.0f);
}

void GameStateMachine::ServerBrowserListRender(Widget* _widget, BlittableRect** _rect, std::string _name)
{
	string s;
	*_rect = new BlittableRect("ServerIcon.png");

	boost::char_separator<char> sep(":");
	boost::tokenizer<boost::char_separator<char> > tokens(_name, sep);

	std::vector<std::string> lines;
	for (boost::tokenizer<boost::char_separator<char> >::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
	{
		//Maxiumum line length is (78 - 16) / 10 = 7 characters
		int len_used = 0;
		while(len_used < tok_iter->length())
		{
			int start = len_used;
			len_used += 7;
			if(len_used > tok_iter->length()) len_used = tok_iter->length();

			string substr = tok_iter->substr(start, len_used);
			lines.push_back(substr);
		}
		
	}

	(*_rect)->SetTextSize(TextSize::Small);
	(*_rect)->BlitTextLines(lines, TextAlignment::Centre);
	
	//TODO overlay favourites here
	/*SDLAnimationFrame::screen_ = surface;
	pl.grid_animation_->GetCurrentFrame()->Draw(Vector2f(3, 3));
	if(Progress::GetProgress(rel_path_ + "/" + _name).completed)
	{
		BlittableRect overlay("Tick.png");
		overlay.Blit(Vector2i(surface->w - overlay.GetSize().x - 4, surface->h - overlay.GetSize().y - 4), *_rect);
	}*/
}

/* Multiplayer */
void GameStateMachine::SetupMultiplayer()
{
}

void GameStateMachine::ProcessMultiplayer(float _timespan)
{
}

void GameStateMachine::TeardownMultiplayer()
{
	Widget::ClearRoot();
}


/* Multiplayer event handling */

/* Main methods */
void GameStateMachine::CreateRenderArea(Vector2i _level_size, Mode::Enum _mode_affected)
{
	if(render_area_)
	{
		SDL_FreeSurface(render_area_);
	}
	//Limit to screen size
	Vector2i render_area_size = _level_size;
	size_ = _level_size;
	int max_x = (SDL_GetVideoSurface()->w - 138) / Settings::GetGridSize().x;
	int max_y = (SDL_GetVideoSurface()->h - 74) / Settings::GetGridSize().y;
	if(render_area_size.x > max_x)
		render_area_size.x = max_x;
	if(render_area_size.y > max_y)
		render_area_size.y = max_y;

	scroll_limit_ = _level_size - render_area_size;
	switch(_mode_affected)
	{
	case Mode::Puzzle: //TODO puzzle scrolling
		puzzle_level_->SetScrollLimit(scroll_limit_);
		break;
	case Mode::Editor:
		editor_level_->SetScrollLimit(scroll_limit_);
		break;
	}


	SDL_Surface* tsurface = SDL_CreateRGBSurface(SDL_GetVideoSurface()->flags,
		Settings::GetGridSize().x * render_area_size.x + 1,
		Settings::GetGridSize().y * render_area_size.y + 1,
		SDL_GetVideoSurface()->format->BytesPerPixel * 8,
		SDL_GetVideoSurface()->format->Rmask, 
		SDL_GetVideoSurface()->format->Gmask, 
		SDL_GetVideoSurface()->format->Bmask, 
		SDL_GetVideoSurface()->format->Amask);
	render_area_ = SDL_DisplayFormatAlpha(tsurface);
	SDL_FreeSurface(tsurface);

	if(scroll_left_widget_)
		scroll_left_widget_->Delete();
	if(scroll_right_widget_)
		scroll_right_widget_->Delete();
	if(scroll_up_widget_)
		scroll_up_widget_->Delete();
	if(scroll_down_widget_)
		scroll_down_widget_->Delete();

	scroll_left_widget_ = new Widget(VerticalTile("Scroll_LeftTop.png", "Scroll_LeftCentre.png", "Scroll_LeftBottom.png"), Settings::GetGridSize().y * render_area_size.y);
	scroll_left_widget_->SetPosition(Vector2i(128, 10));
	scroll_left_widget_->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollLeftClick, this, _1));
	scroll_right_widget_ = new Widget(VerticalTile("Scroll_RightTop.png", "Scroll_RightCentre.png", "Scroll_RightBottom.png"), Settings::GetGridSize().y * render_area_size.y);
	scroll_right_widget_->SetPosition(Vector2i(render_area_size.x * Settings::GetGridSize().x + 138 + 1, 10));
	scroll_right_widget_->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollRightClick, this, _1));
	scroll_down_widget_ = new Widget(HorizontalTile("Scroll_DownLeft.png", "Scroll_DownCentre.png", "Scroll_DownRight.png"), Settings::GetGridSize().x * render_area_size.x);
	scroll_down_widget_->SetPosition(Vector2i(138, render_area_size.y * Settings::GetGridSize().y + 11));
	scroll_down_widget_->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollDownClick, this, _1));
	scroll_up_widget_ = new Widget(HorizontalTile("Scroll_UpLeft.png", "Scroll_UpCentre.png", "Scroll_UpRight.png"), Settings::GetGridSize().x * render_area_size.x);
	scroll_up_widget_->SetPosition(Vector2i(138, 0));
	scroll_up_widget_->OnClick.connect(boost::bind(&GameStateMachine::PuzzleScrollUpClick, this, _1));


	GameGridWidget* grid_widget = new GameGridWidget(render_area_size, Vector2i(32, 32));
	grid_widget->SetPosition(Vector2i(138, 10));
	grid_widget->OnGridClick.connect(boost::bind(&GameStateMachine::PuzzleGridClick, this, _1, _2));
	grid_widget->OnGridGesture.connect(boost::bind(&GameStateMachine::PuzzleGridGesture, this, _1, _2));
	grid_widget->OnMouseMove.connect(boost::bind(&GameStateMachine::GamegridMouseMoveCallback, this, _1, _2));
}

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
		case Mode::ServerBrowser:
			TeardownServerBrowser();
			break;
		case Mode::Multiplayer:
			TeardownMultiplayer();
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
		case Mode::ServerBrowser:
			SetupServerBrowser();
			break;
		case Mode::Multiplayer:
			SetupMultiplayer();
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
	case Mode::ServerBrowser:
		ProcessServerBrowser(_timespan);
		break;
	case Mode::Multiplayer:
		ProcessMultiplayer(_timespan);
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

	if(render_area_)
	{
		SDL_FillRect(render_area_, NULL, SDL_MapRGBA(render_area_->format, 0, 0, 0, 0));
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
		SDLAnimationFrame::screen_ = render_area_;


		BOOST_FOREACH(RenderItem& ri, render_items)
		{
			ri.frame_->Draw(ri.position_);
			if(ri.position_.x < 0)
				ri.frame_->Draw(ri.position_ + Vector2f(static_cast<float>(Settings::GetGridSize().x * size_.x), 0));
			if(ri.position_.x > Settings::GetGridSize().x * size_.x - Settings::GetGridSize().x)
				ri.frame_->Draw(Vector2f(ri.position_.x - Settings::GetGridSize().x * size_.x, ri.position_.y));
			if(ri.position_.y < 0)
				ri.frame_->Draw(ri.position_ + Vector2f(0, static_cast<float>(Settings::GetGridSize().y * size_.y)));
			if(ri.position_.y > Settings::GetGridSize().y * size_.y - Settings::GetGridSize().y)
				ri.frame_->Draw(Vector2f(ri.position_.x, ri.position_.y - Settings::GetGridSize().y * size_.y));
		}
		SDL_Rect game_area_rect;
		game_area_rect.x = 138;
		game_area_rect.y = 10;
		game_area_rect.w = 0;
		game_area_rect.h = 0;

		SDL_BlitSurface(render_area_, NULL, _target, &game_area_rect);	
	}
	//Render small indicator area
	{
		std::vector<RenderItem> render_items;
				switch(mode_)
		{
		case Mode::Editor:
		case Mode::Puzzle:
			if(state_indicator_level_.get() != NULL)
			{
				render_items = state_indicator_level_->Draw();
			}
			break;
		}
		//Sort front to back to prevent overlay issues
		std::sort(render_items.begin(), render_items.end(), RenderItem::DepthSort<RenderItem>());
		SDLAnimationFrame::screen_ = _target;
		BOOST_FOREACH(RenderItem& ri, render_items)
		{
			ri.frame_->Draw(ri.position_ + Vector2i(16, 378));
		}
	}
}

void GameStateMachine::FadeInOut(float _total_time)
{
	total_fade_time_ = _total_time;
	fade_timer_ = _total_time;
}

void GameStateMachine::LayoutArrows(std::vector<Direction::Enum> _arrows)
{
	arrow_stock_widget_->ClearChildren();
	int arrow_id = 0;
	std::string arrow_filenames[5] = {"NorthA1.png", "SouthA1.png", "EastA1.png", "WestA1.png", "WestA1.png"};
	std::string arrow_set_filenames[8][5] = {{"NorthA3.png", "SouthA3.png", "EastA3.png", "WestA3.png", "WestA1.png"},
											 {"NorthA4.png", "SouthA4.png", "EastA4.png", "WestA4.png", "WestA1.png"},
											 {"NorthA5.png", "SouthA5.png", "EastA5.png", "WestA5.png", "WestA1.png"},
											 {"NorthA6.png", "SouthA6.png", "EastA6.png", "WestA6.png", "WestA1.png"},
											 {"NorthA7.png", "SouthA7.png", "EastA7.png", "WestA7.png", "WestA1.png"},
											 {"NorthA8.png", "SouthA8.png", "EastA8.png", "WestA8.png", "WestA1.png"},
											 {"NorthA9.png", "SouthA9.png", "EastA9.png", "WestA9.png", "WestA1.png"},
											 {"NorthAm.png", "SouthAm.png", "EastAm.png", "WestAm.png", "WestA1.png"},};

	int arrow_count[5] = {0, 0, 0, 0, 0};
	BOOST_FOREACH(Direction::Enum arrow, _arrows)
	{
		arrow_count[arrow]++;
	}
	for(int arrow_dir = 0; arrow_dir < 5; arrow_dir++)
	{
		if(arrow_count[arrow_dir] >= 3)
		{
			int index = arrow_count[arrow_dir] - 3;
			if(arrow_count[arrow_dir] > 9)
				index = 7;
			Widget* arrow_widget = new Widget(arrow_set_filenames[index][arrow_dir]);
			arrow_widget->SetPosition(Vector2i(16 + arrow_id * Settings::GetGridSize().x, 8));
			arrow_widget->SetRejectsFocus(true);
			arrow_stock_widget_->AddChild(arrow_widget);
			arrow_id += 2;
		} else
		{
			for(int i = 0; i < arrow_count[arrow_dir]; i++)
			{
				Widget* arrow_widget = new Widget(arrow_filenames[arrow_dir]);
				arrow_widget->SetPosition(Vector2i(16 + arrow_id * Settings::GetGridSize().x, 8));
				arrow_widget->SetRejectsFocus(true);
				arrow_stock_widget_->AddChild(arrow_widget);
				arrow_id++;
			}
		}
	}
}



void GameStateMachine::KeyboardCallback(Widget* /*_widget*/, KeyPressEventArgs _args)
{
	if(!_args.key_up)
		return;
	switch(_args.key_code)
	{
	case SDLK_w:
		input_.action = Action::PlaceNorthArrow;
		input_.position = last_grid_position_;
		break;
	case SDLK_a:
		input_.action = Action::PlaceWestArrow;
		input_.position = last_grid_position_;
		break;
	case SDLK_s:
		input_.action = Action::PlaceSouthArrow;
		input_.position = last_grid_position_;
		break;
	case SDLK_d:
		input_.action = Action::PlaceEastArrow;
		input_.position = last_grid_position_;
		break;
	case SDLK_SPACE:
		input_.action = Action::Start;
		break;
	case SDLK_LEFT:
		input_.action = Action::ScrollWest;
		break;
	case SDLK_RIGHT:
		input_.action = Action::ScrollEast;
		break;
	case SDLK_UP:
		input_.action = Action::ScrollNorth;
		break;
	case SDLK_DOWN:
		input_.action = Action::ScrollSouth;
		break;
	case SDLK_ESCAPE:
	case SDLK_BACKSPACE:
		input_.action = Action::Cancel;
		break;
	}
}

void GameStateMachine::GamegridMouseMoveCallback(Widget* /*_widget*/, MouseEventArgs _args)
{
	last_grid_position_.x = _args.x / Settings::GetGridSize().x;
	last_grid_position_.y = _args.y / Settings::GetGridSize().y;
}