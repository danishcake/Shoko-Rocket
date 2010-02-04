#include "Settings.h"
#include <TinyXML.h>
#include "Logger.h"
#include <boost/lexical_cast.hpp>
#include <string>
using std::string;

Color::Color(unsigned char _r, unsigned char _g, unsigned char _b)
{
	r = _r;
	g = _g;
	b = _b;
}
Color::Color()
{
	r = 0;
	g = 0;
	b = 0;
}


Settings::Settings(void)
{
	grid_size_ = Vector2i(32, 32);
	resolution_ = Vector2i(640, 480);
	grid_color_a = Color(247, 215, 168);
	grid_color_b = Color(172, 149, 240);
	mouse_sprite_ = "Mouse_ShokoWhite.animation";
	cat_sprite_ = "KapuKapu.animation";
	hole_sprite_ = "Hole.animation";
	rocket_sprite_ = "Rocket.animation";
	arrows_sprite_ = "Arrows.animation";
	half_arrows_sprite_ = "HalfArrows.animation";
	arrow_sets_ = "ArrowSets.animation";
	ring_sprite_ = "Ring.animation";
	use_gestures_ = true;


	TiXmlDocument doc("Settings.xml");
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("Settings");
	if(root)
	{
		TiXmlElement* grid_size = root->FirstChildElement("GridSize");
		TiXmlElement* resolution = root->FirstChildElement("Resolution");
		TiXmlElement* mouse_animation = root->FirstChildElement("MouseAnimation");
		TiXmlElement* cat_animation = root->FirstChildElement("CatAnimation");
		TiXmlElement* rocket_animation = root->FirstChildElement("RocketAnimation");
		TiXmlElement* hole_animation = root->FirstChildElement("HoleAnimation");
		TiXmlElement* arrows_animation = root->FirstChildElement("ArrowsAnimation");
		TiXmlElement* half_arrows_animation = root->FirstChildElement("HalfArrowsAnimation");
		TiXmlElement* arrow_sets_animation = root->FirstChildElement("ArrowSetsAnimation");
		TiXmlElement* ring_animation = root->FirstChildElement("RingAnimation");
		TiXmlElement* grid_color_a_el = root->FirstChildElement("GridColorA");
		TiXmlElement* grid_color_b_el = root->FirstChildElement("GridColorB");
		TiXmlElement* use_gestures_el = root->FirstChildElement("UseGestures");


		if(!grid_size || (grid_size->QueryIntAttribute("x", &grid_size_.x) != TIXML_SUCCESS) ||
						 (grid_size->QueryIntAttribute("y", &grid_size_.y) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to load grid size, defaulting to 32x32\n";

		if(!resolution || (resolution->QueryIntAttribute("x", &resolution_.x) != TIXML_SUCCESS) ||
						  (resolution->QueryIntAttribute("y", &resolution_.y) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to load resolution, defaulting to 640x480\n";

		if(!mouse_animation || (mouse_animation->QueryValueAttribute("File", &mouse_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse MouseAnimation, defaulting to Mouse_ShokoWhite.animation\n";

		if(!cat_animation || (cat_animation->QueryValueAttribute("File", &cat_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse CatAnimation, defaulting to KapuKapu.animation\n";

		if(!rocket_animation || (rocket_animation->QueryValueAttribute("File", &rocket_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse RocketAnimation, defaulting to Rocket.animation\n";

		if(!hole_animation || (hole_animation->QueryValueAttribute("File", &hole_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse HoleAnimation, defaulting to Hole.animation\n";

		if(!ring_animation || (ring_animation->QueryValueAttribute("File", &ring_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse RingAnimation, defaulting to Ring.animation\n";

		if(!arrows_animation || (arrows_animation->QueryValueAttribute("File", &arrows_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse ArrowsAnimation, defaulting to Arrows.animation\n";
		
		if(!half_arrows_animation || (half_arrows_animation->QueryValueAttribute("File", &half_arrows_sprite_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse HalfArrowsAnimation, defaulting to HalfArrows.animation\n";

		if(!arrow_sets_animation || (arrow_sets_animation->QueryValueAttribute("File", &arrow_sets_) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse ArrowSetsAnimation, defaulting to ArrowSets.animation\n";

		int r = 247;
		int g = 215;
		int b = 168;

		if(!grid_color_a_el|| (grid_color_a_el->QueryValueAttribute("r", &r) != TIXML_SUCCESS) ||
							(grid_color_a_el->QueryValueAttribute("g", &g) != TIXML_SUCCESS) ||
							(grid_color_a_el->QueryValueAttribute("b", &b) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse GridColorA\n";
		else
		{
			grid_color_a.r = static_cast<unsigned char>(r);
			grid_color_a.g = static_cast<unsigned char>(g);
			grid_color_a.b = static_cast<unsigned char>(b);
		}

		r = 172;
		g = 149;
		b = 240;

		if(!grid_color_b_el || (grid_color_b_el->QueryValueAttribute("r", &r) != TIXML_SUCCESS) ||
							   (grid_color_b_el->QueryValueAttribute("g", &g) != TIXML_SUCCESS) ||
							   (grid_color_b_el->QueryValueAttribute("b", &b) != TIXML_SUCCESS))
			Logger::DiagnosticOut() << "Unable to find or parse GridColorB\n";
		else
		{
			grid_color_b.r = static_cast<unsigned char>(r);
			grid_color_b.g = static_cast<unsigned char>(g);
			grid_color_b.b = static_cast<unsigned char>(b);
		}

		if(!use_gestures_el || use_gestures_el->QueryValueAttribute("Use", &use_gestures_) != TIXML_SUCCESS)
			Logger::DiagnosticOut() << "Unable to find or parse UseGestures\n";
	}

}

Settings::~Settings(void)
{
	TiXmlDocument doc("Settings.xml");
	doc.Clear();
	TiXmlDeclaration* decl = new TiXmlDeclaration();
	doc.LinkEndChild(decl);

	TiXmlElement* root= new TiXmlElement("Settings");
	TiXmlElement* grid_size = new TiXmlElement("GridSize");
	grid_size->SetAttribute("x", boost::lexical_cast<string, int>(grid_size_.x));
	grid_size->SetAttribute("y", boost::lexical_cast<string, int>(grid_size_.y));
	TiXmlElement* resolution = new TiXmlElement("Resolution");
	resolution->SetAttribute("x", boost::lexical_cast<string, int>(resolution_.x));
	resolution->SetAttribute("y", boost::lexical_cast<string, int>(resolution_.y));
	TiXmlElement* mouse_animation_el = new TiXmlElement("MouseAnimation");
	mouse_animation_el->SetAttribute("File", mouse_sprite_);
	TiXmlElement* cat_animation_el = new TiXmlElement("CatAnimation");
	cat_animation_el->SetAttribute("File", cat_sprite_);
	TiXmlElement* rocket_animation = new TiXmlElement("RocketAnimation");
	rocket_animation->SetAttribute("File",rocket_sprite_);
	TiXmlElement* hole_animation = new TiXmlElement("HoleAnimation");
	hole_animation->SetAttribute("File", hole_sprite_);
	TiXmlElement* grid_color_a_el = new TiXmlElement("GridColorA");
	grid_color_a_el->SetAttribute("r", grid_color_a.r);
	grid_color_a_el->SetAttribute("g", grid_color_a.g);
	grid_color_a_el->SetAttribute("b", grid_color_a.b);
	TiXmlElement* grid_color_b_el = new TiXmlElement("GridColorB");
	grid_color_b_el->SetAttribute("r", grid_color_b.r);
	grid_color_b_el->SetAttribute("g", grid_color_b.g);
	grid_color_b_el->SetAttribute("b", grid_color_b.b);
	TiXmlElement* use_gestures_el = new TiXmlElement("UseGestures");
	use_gestures_el->SetAttribute("Use", use_gestures_);

	TiXmlElement* arrows_animation = new TiXmlElement("ArrowsAnimation");
	arrows_animation->SetAttribute("File", arrows_sprite_);
	TiXmlElement* half_arrows_animation = new TiXmlElement("HalfArrowsAnimation");
	half_arrows_animation->SetAttribute("File", half_arrows_sprite_);
	TiXmlElement* arrow_sets_animation = new TiXmlElement("ArrowSetsAnimation");
	arrow_sets_animation->SetAttribute("File", arrow_sets_);
	TiXmlElement* ring_animation = new TiXmlElement("RingAnimation");
	ring_animation->SetAttribute("File", ring_sprite_);


	root->LinkEndChild(grid_size);
	root->LinkEndChild(resolution);
	root->LinkEndChild(mouse_animation_el);
	root->LinkEndChild(cat_animation_el);
	root->LinkEndChild(rocket_animation);
	root->LinkEndChild(hole_animation);
	root->LinkEndChild(grid_color_a_el);
	root->LinkEndChild(grid_color_b_el);
	root->LinkEndChild(use_gestures_el);
	root->LinkEndChild(arrows_animation);
	root->LinkEndChild(half_arrows_animation);
	root->LinkEndChild(arrow_sets_animation);
	root->LinkEndChild(ring_animation);

	//TODO save new settings
	doc.LinkEndChild(root);

	doc.SaveFile();
}

Settings& Settings::GetInstance()
{
	static Settings settings_instance;
	return settings_instance;
}

Vector2i Settings::GetGridSize()
{
	return GetInstance().grid_size_;
}

Vector2i Settings::GetResolution()
{
	return GetInstance().resolution_;
}

Color Settings::GetGridColorA()
{
	return GetInstance().grid_color_a;
}

Color Settings::GetGridColorB()
{
	return GetInstance().grid_color_b;
}

string Settings::GetMouseSprite()
{
	return GetInstance().mouse_sprite_;
}

string Settings::GetCatSprite()
{
	return GetInstance().cat_sprite_;
}

string Settings::GetHoleSprite()
{
	return GetInstance().hole_sprite_;
}

string Settings::GetRocketSprite()
{
	return GetInstance().rocket_sprite_;
}

bool Settings::GetUseGestures()
{
	return GetInstance().use_gestures_;
}

void Settings::SetUseGestures(bool _use)
{
	GetInstance().use_gestures_ = _use;
}

std::string Settings::GetArrowsSprite()
{
	return GetInstance().arrows_sprite_;
}

std::string Settings::GetHalfArrowsSprite()
{
	return GetInstance().half_arrows_sprite_;
}

std::string Settings::GetArrowSets()
{
	return GetInstance().arrow_sets_;
}

std::string Settings::GetRingSprite()
{
	return GetInstance().ring_sprite_;
}