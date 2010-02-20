#pragma once
#include "BaseLevel.h"

namespace EditMode
{
	enum Enum
	{
		EditArrows, EditWalls, EditCats, EditMice, EditHoles, EditRockets,
		Running, RunningFast, Validated, Invalidated
	};
}

class EditLevel : public BaseLevel
{
protected:
	EditMode::Enum edit_mode_;
	EditMode::Enum edit_mode_restore_;
	bool validated_;

public:
	EditLevel(Vector2i _level_size, Vector2f _grid_size);
	virtual ~EditLevel(void);

	virtual void Tick(float _time, Input _input);
	EditMode::Enum GetEditMode(){return edit_mode_;}
	void SetEditMode(EditMode::Enum _edit_mode);
	bool GetValidated(){return validated_;}
	void SetName(std::string _level_name){world_->SetName(_level_name);}
	std::string GetName(){return world_->GetName();}
	void Save(std::string _name);
};
