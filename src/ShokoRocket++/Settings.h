#pragma once
#include "vmath.h"
#include <string>

struct Color
{
public:
	Color(unsigned char _r, unsigned char _g, unsigned char _b);
	Color();
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

class Settings
{
private:
	Settings(void);
	static Settings& GetInstance();
	
	Vector2i grid_size_;
	Vector2i resolution_;
	bool full_screen_;
	Color grid_color_a;
	Color grid_color_b;
	std::string mouse_sprite_;
	std::string cat_sprite_;
	std::string hole_sprite_;
	std::string rocket_sprite_;
	std::string arrows_sprite_;
	std::string half_arrows_sprite_;
	std::string arrow_sets_;
	std::string ring_sprite_;
	bool use_gestures_;

public:
	static Vector2i GetGridSize();
	static Vector2i GetResolution();
	static bool GetFullScreen();
	static Color GetGridColorA();
	static Color GetGridColorB();
	static std::string GetMouseSprite();
	static std::string GetCatSprite();
	static std::string GetHoleSprite();
	static std::string GetRocketSprite();
	static std::string GetArrowsSprite();
	static std::string GetHalfArrowsSprite();
	static std::string GetRingSprite();
	static std::string GetArrowSets();
	
	static bool GetUseGestures();
	static void SetUseGestures(bool _use);

	~Settings(void);


};
