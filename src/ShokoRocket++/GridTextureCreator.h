#pragma once
#include <boost/shared_ptr.hpp>
#include <World.h>
class Animation;


Animation* CreateGridTexture(boost::shared_ptr<World> _world, Vector2i _grid_size);