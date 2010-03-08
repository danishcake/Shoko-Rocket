struct SDL_Surface;
class Widget;
class ItemBrowserWidget;
class BlittableRect;
#include <string>
#include "PuzzleLevel.h"
#include "EditLevel.h"
#include "StatusLevel.h"
#include "boost/shared_ptr.hpp"
#include <event.h>
#include <GameGridWidget.h>

class Client;
class Server;
class MPWorld;
class ServerWorld;

namespace Mode
{
	enum Enum
	{
		Intro, Menu, Puzzle, Editor, ServerBrowser, Lobby, Multiplayer, Exit
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
	Widget* edit_widget_;

	ItemBrowserWidget* levels_widget_;
	std::string rel_path_;
	bool reload_due_;
	std::string selected_level_;
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
	void MenuEditExistingCallback(Widget* _widget);
	void MenuOptionsCallback(Widget* _widget);
	void MenuLevelBrowerPageChange(Widget* _widget, int _old_page, int _new_page);
	void MenuPlayOnlineCallback(Widget* _widget);
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
	bool create_new_level_;
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

	
	/* Common multiplayer members */
	Client* client_;
	Server* server_;
	MPWorld* client_world_;
	ServerWorld* server_world_;
	std::map<int, std::string> player_names_;

	/* ServerBrowser members */
	Widget* ip_area_;
	Widget* port_area_;
	Widget* join_server_;
	Widget* start_server_;
	/* ServerBrowser methods */
	void SetupServerBrowser();
	void ProcessServerBrowser(float _timespan);
	void TeardownServerBrowser();
	/* ServerBrowser event handling */
	void ServerBrowserBackCallback(Widget* _widget);
	void ServerBrowserListRender(Widget* _widget, BlittableRect** _rect, std::string _name);
	void ServerBrowserItemHighlightCallback(Widget* _widget, std::string _name);
	void JoinServerCallback(Widget* _widget);
	void StartServerCallback(Widget* _widget);

	/* Lobby members */
	Widget* chat_widget_;
	std::string chat_hist_;
	/* Lobby methods */
	void SetupLobby();
	void ProcessLobby(float _timespan);
	void TeardownLobby();
	/* Lobby event handling */
	void LobbyReturnToBrowser(Widget* _widget);
	void LobbyChatEntry(Widget* _widget, KeyPressEventArgs _event_args);
	void LobbyNameChange(Widget* _widget);

	/* Multiplayer members */
	/* Multiplayer methods */
	void SetupMultiplayer();
	void ProcessMultiplayer(float _timespan);
	void TeardownMultiplayer();
	/* Multiplayer event handling */

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

	boost::shared_ptr<StatusLevel> state_indicator_level_;
public:
	/* Constructors */
	GameStateMachine();
	~GameStateMachine();

	/* Main tick routine */
	bool Tick(float _timespan); //Returns true when exitting
	void Draw(SDL_Surface* _target);

};