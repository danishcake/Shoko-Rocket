#pragma once
struct SDL_Surface;
#include <boost/shared_ptr.hpp>
#include <World.h>
#include "RenderItem.h"
#include "Input.h"
#include <vector>
#include <string>


class Animation;

namespace EditMode
{
	enum Enum
	{
		EditArrows, EditWalls, EditCats, EditMice, EditHoles, EditRockets,
		Running, RunningFast, Validated, Invalidated
	};
}

class EditLevel
{
protected:
	boost::shared_ptr<World> world_;
	EditMode::Enum edit_mode_;
	EditMode::Enum edit_mode_restore_;
	float timer_;
	Vector2f grid_size_;
	Vector2f scroll_offset_;
	Vector2i scroll_limit_;
	Vector2f scroll_remaining_;
	bool validated_;

public:
	EditLevel(Vector2i _level_size, Vector2f _grid_size);
	~EditLevel(void);

	std::vector<RenderItem> Draw();
	void Tick(float _time, Input _input);
	EditMode::Enum GetEditMode(){return edit_mode_;}
	void SetEditMode(EditMode::Enum _edit_mode);
	bool GetValidated(){return validated_;}
	void SetName(std::string _level_name){world_->SetName(_level_name);}
	std::string GetName(){return world_->GetName();}
	void Save(std::string _name);
	Animation* grid_animation_;
	Vector2i GetLevelSize(){return world_->GetSize();}
	void SetScrollLimit(Vector2i _scroll_limit){scroll_limit_ = _scroll_limit;}
};
