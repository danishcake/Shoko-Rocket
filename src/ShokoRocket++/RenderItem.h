#pragma once

class AnimationFrame;
#include <functional>
#include <vmath.h>

struct RenderItem
{
public:
	AnimationFrame* frame_;
	Vector2f position_;
	float depth;

	template <class list_t> 
	class DepthSort: public std::binary_function<list_t, list_t, bool> 
	{
	public:
		DepthSort(){};

		bool operator()(list_t a, list_t b) const
		{
			return (a.position_.y + a.depth) < (b.position_.y + b.depth);
		}
	};
};