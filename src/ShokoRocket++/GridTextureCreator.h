#pragma once
#include <boost/shared_ptr.hpp>
#include <BaseWorld.h>
class Animation;


Animation* CreateGridTexture(boost::shared_ptr<BaseWorld> _world, Vector2i _grid_size);