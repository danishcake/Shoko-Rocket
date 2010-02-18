struct SDL_Surface;
class Widget;
class ItemBrowserWidget;
class BlittableRect;
#include <string>
#include "PuzzleLevel.h"
#include "EditLevel.h"
#include "boost/shared_ptr.hpp"
#include <event.h>
#include <GameGridWidget.h>

namespace Mode
{
	enum Enum
	{
		Intro, Menu, Puzzle, Editor, Exit
	};
}

namespace Submode
{
	enum Enum
	{
		None, LevelList, Options
	};
}

namespace InputMethod
{
	enum Enum
	{
		MouseClicks, MouseDrags
	};
}

class GameStateMachine
{
private:
	/* Mode handling */
	float mode_timer_;
	float sub_mode_timer_;
	Mode::Enum mode_;
	Mode::Enum pend_mode_;
	Submode::Enum sub_mode_;
	Submode::Enum pend_sub_mode_;
	InputMethod::Enum input_method_;
	bool level_completed_;
	int page_of_level_;
	PuzzleMode::Enum last_puzzle_mode_;
	std::vector<std::string> puzzle_files_;
	int puzzle_index_;

	/* File query methods */
	void LoadPuzzleFiles();

	/* Intro methods */
	void SetupIntro();
	void ProcessIntro(float _timespan);
	void TeardownIntro();

	/* Menu members */
	Widget* options_widget_;
	Widget* gesture_hint_;
	Widget* clicks_hint_;
	Widget* level_name_;

	ItemBrowserWidget* levels_widget_;
	std::string rel_path_;
	bool reload_due_;
	/*Menu methods */
	void SetupMenu();
	void ProcessMenu(float _timespan);
	void TeardownMenu();
	/* Menu event handling */
	void RenderLevel(Widget* _widget, BlittableRect** _rect, std::string _name);
	void MenuLevelSelect(Widget* _widget, std::string _name);
	void MenuLevelHighlight(Widget* _widget, std::string _name);
	void MenuExitCallback(Widget* _widget);
	void MenuPuzzleCallback(Widget* _widget);
	void MenuEditorCallback(Widget* _widget);
	void MenuOptionsCallback(Widget* _widget);
	void MenuLevelBrowerPageChange(Widget* _widget, int _old_page, int _new_page);
	void OptionsInputmethodCallback(Widget* _widget);

	/* Puzzle members */
	boost::shared_ptr<PuzzleLevel> puzzle_level_;
	Input input_;
	Widget* puzzle_complete_widget_;
	Widget* arrow_stock_widget_;
	int arrow_hash_;
	/* Puzzle methods */
	void SetupPuzzle();
	void ProcessPuzzle(float _timespan);
	void TeardownPuzzle();
	
	void LayoutArrows(std::vector<Direction::Enum> _arrows);

	/* Puzzle event handling */
	void PuzzleStartClick(Widget* _widget);
	void PuzzleResetClick(Widget* _widget);
	void PuzzleQuitClick(Widget* _widget);
	void PuzzleGridClick(Widget* _widget, MouseEventArgs _args);
	void PuzzleGridGesture(Widget* _widget, GridGestureEventArgs _args);
	void PuzzleNextClick(Widget* _widget);
	void PuzzleScrollDownClick(Widget* _widget);
	void PuzzleScrollUpClick(Widget* _widget);
	void PuzzleScrollLeftClick(Widget* _widget);
	void PuzzleScrollRightClick(Widget* _widget);

	/* Editor members */
	boost::shared_ptr<EditLevel> editor_level_;
	Widget* new_level_widget_;
	Vector2i size_;
	Widget* sizex_;
	Widget* sizey_;
	Widget* level_name_editor_;
	/* Editor methods */
	void SetupEditor();
	void ProcessEditor(float _timespan);
	void TeardownEditor();
	/* Edit event handling */
	void EditorNewClick(Widget* _widget);
	void EditorStartClick(Widget* _widget);
	void EditorResetClick(Widget* _widget);
	void EditorSaveClick(Widget* _widget);
	void EditorReturnClick(Widget* _widget);
	void EditorSizePPXClick(Widget* _widget);
	void EditorSizeMMXClick(Widget* _widget);
	void EditorSizePPYClick(Widget* _widget);
	void EditorSizeMMYClick(Widget* _widget);
	void EditorCreateClick(Widget* _widget);

	void EditorWallMode(Widget* _widget);
	void EditorMouseMode(Widget* _widget);
	void EditorCatMode(Widget* _widget);
	void EditorHoleMode(Widget* _widget);
	void EditorRocketMode(Widget* _widget);
	void EditorArrowMode(Widget* _widget);
	

	/* Transition times */
	static const float sub_mode_widget_transition_time;

	/* Fading */
	float total_fade_time_;
	float fade_timer_;
	void FadeInOut(float _total_time);

	/* Common */
	SDL_Surface* render_area_;
	void CreateRenderArea(Vector2i _level_size, Mode::Enum);
	Vector2i scroll_limit_;
	void KeyboardCallback(Widget* _widget, KeyPressEventArgs _args);
	void GamegridMouseMoveCallback(Widget* _widget, MouseEventArgs _args);
	Vector2i last_grid_position_;

	Widget* scroll_left_widget_;
	Widget* scroll_right_widget_;
	Widget* scroll_up_widget_;
	Widget* scroll_down_widget_;

public:
	/* Constructors */
	GameStateMachine();
	~GameStateMachine();

	/* Main tick routine */
	bool Tick(float _timespan); //Returns true when exitting
	void Draw(SDL_Surface* _target);

};