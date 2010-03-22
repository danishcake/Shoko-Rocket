#pragma once
#include <MPWorld.h>
#include "RenderItem.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include "vmath.h"
#include <Opcodes.h>

class Animation;

class MPLevel
{
protected:
	boost::shared_ptr<MPWorld> world_;
	
	float timer_;
	Vector2f grid_size_;
	Vector2f scroll_remaining_;
	Vector2f scroll_offset_;
	Vector2i scroll_limit_;
public:
	MPLevel(Vector2f _grid_size, MPWorld* _world);
	~MPLevel(void);

	void HandleOpcodes(std::vector<Opcodes::ServerOpcode*> _opcodes);
	void Tick(float _dt);
	std::vector<RenderItem> Draw();
	
	std::string GetFilename(){return world_->GetFilename();}
	Vector2i GetLevelSize(){return world_->GetSize();}
	void SetScrollLimit(Vector2i _scroll_limit){scroll_limit_ = _scroll_limit;}
	Animation* grid_animation_;


};
