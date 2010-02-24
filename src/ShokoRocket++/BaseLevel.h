#pragma once
struct SDL_Surface;
#include <SPWorld.h>
#include "RenderItem.h"
#include "Input.h"
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

class Animation;

class BaseLevel
{
protected:
	boost::shared_ptr<SPWorld> world_;
	float timer_;
	Vector2f grid_size_;
	Vector2f scroll_remaining_;
	Vector2f scroll_offset_;
	Vector2i scroll_limit_;
public:
	BaseLevel(Vector2f _grid_size);
	virtual ~BaseLevel();
	virtual std::vector<RenderItem> Draw();
	virtual void Tick(float _time, Input _input);

	std::string GetFilename(){return world_->GetFilename();}
	Vector2i GetLevelSize(){return world_->GetSize();}
	
	std::vector<Direction::Enum> GetArrows(){return world_->GetArrows();}
	int ComputeArrowHash();
	void SetScrollLimit(Vector2i _scroll_limit){scroll_limit_ = _scroll_limit;}
	Animation* grid_animation_;
};
