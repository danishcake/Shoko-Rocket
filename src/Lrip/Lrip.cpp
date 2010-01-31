// Lrip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <tinyxml.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
using std::string;
using boost::lexical_cast;


static char translation_lookup[] = 
{
' ', '!', '"', '0', '0', //0-4
'0', '0', '\'', '0', '0', //5-9
'0', '0', ',', '-', '0', //10-14
'0', '0', '1', '2', '3', //15-19
'4', '5', '6', '7', '8', //20-24
'9', '?', '?', '?', '?', //25-29
'?', '?', '?', 'A', 'B', //30-34
'C', 'D', 'E', 'F', 'G', //35-39
'H', 'I', 'J', 'K', 'L', //40-44
'M', 'N', 'O', 'P', 'Q', //45-49
'R', 'S', 'T', 'U', 'V', //50-54
'W', 'X', 'Y', 'Z', '?', //55-59
'?', '?', '?', '?', '?', //60-64
'a', 'b', 'c', 'd', 'e', //65-69
'f', 'g', 'h', 'i', 'j', //70-74
'k', 'l', 'm', 'n', 'o', //75+
'p', 'q', 'r', 's', 't', //75+
'u', 'v', 'w', 'x', 'y', //75+
'z', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+
'?', '?', '?', '?', '?', //75+

};

struct GridItem
{
	/* Split into hex nibbles */
	unsigned char direction : 2; //1   = 0100 = 0000 00xx 0000 0000
	//unsigned char unknown1 : 1; //1   = 0100 = 0000 0001 0000 0000
	//unsigned char unknown2 : 1; //2   = 0200 = 0000 0010 0000 0000
	unsigned char arrow_direction : 2; //4   = 0x00 = 0000 xx00 0000 0000
	//unsigned char unknown3 : 1; //8   = 0800 = 0000 0100 0000 0000
	//unsigned char unknown4 : 1; //8   = 0800 = 0000 1000 0000 0000

	unsigned char unknown5 : 1; //16  = 1000 = 0001 0000 0000 0000
	unsigned char rocket   : 1; //32  = 2000 = 0010 0000 0000 0000
	unsigned char arrow    : 1; //64  = 4000 = 0100 0000 0000 0000
	unsigned char hole     : 1; //128 = 8000 = 1000 0000 0000 0000

	unsigned char mouse     : 1;
	unsigned char cat       : 1;
	unsigned char unknown10 : 1;
	unsigned char unknown11 : 1;	
	
	unsigned char east : 1;
	unsigned char south : 1;
	unsigned char west : 1;
	unsigned char north : 1;
	
};

struct Level
{
public:
	union
	{
		unsigned char data[216];
		GridItem grid[9][12];
	} level_data;
	char unknown[40];
	char name[40];	
};

struct UserLevel
{
public:
	union
	{
		unsigned char data[216];
		GridItem grid[9][12];
	} level_data;
	char name[40];
};

UserLevel LevelToUserLevel(Level _level)
{
	UserLevel user_level;
	memcpy(&user_level.level_data, &_level.level_data, 216);
	memcpy(&user_level.name, &_level.name, 40);
	return user_level;
}

void dump_level(UserLevel level_data, std::string _name, std::string _offset, std::string _author)
{
	int mouse_count = 0;
	int cat_count = 0;
	int hole_count = 0;
	int rocket_count = 0;
	int arrow_count = 0;
	int unknown_count = 0;

	std::string level_name;
	for(int i =0; i < 40; i++)
	{
		if(i%2 == 0)
			level_name.append( lexical_cast<string, char>(translation_lookup[level_data.name[i]]));
	}
	boost::algorithm::trim_right(level_name);





	TiXmlDocument doc(_name);
	doc.Clear();
	TiXmlDeclaration* decl = new TiXmlDeclaration();
	doc.LinkEndChild(decl);
	TiXmlElement* root = new TiXmlElement("Level");
	doc.LinkEndChild(root);
	TiXmlElement* name = new TiXmlElement("Name");
	name->LinkEndChild(new TiXmlText(level_name));
	root->LinkEndChild(name);
	TiXmlElement* size = new TiXmlElement("Size");
	size->SetAttribute("x", 12);
	size->SetAttribute("y", 9);
	root->LinkEndChild(size);

	TiXmlElement* author = new TiXmlElement("Author");
	author->LinkEndChild(new TiXmlText(_author));
	root->LinkEndChild(author);

	TiXmlElement* source = new TiXmlElement("Source");
	source->LinkEndChild(new TiXmlText("Chu Chu Rocket.gba, offset " + _offset));
	root->LinkEndChild(source);

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 9; y++)
		{
			if(level_data.level_data.grid[y][x].north)
			{
				TiXmlElement* h = new TiXmlElement("H");
				h->SetAttribute("x", x);
				h->SetAttribute("y", y);
				root->LinkEndChild(h);
			}
			if(level_data.level_data.grid[y][x].west)
			{
				TiXmlElement* v = new TiXmlElement("V");
				v->SetAttribute("x", x);
				v->SetAttribute("y", y);
				root->LinkEndChild(v);
			}
			if(level_data.level_data.grid[y][x].rocket)
			{
				TiXmlElement* rocket = new TiXmlElement("Rocket");
				rocket->SetAttribute("x", x);
				rocket->SetAttribute("y", y);
				root->LinkEndChild(rocket);
				rocket_count++;
			}
			if(level_data.level_data.grid[y][x].hole)
			{
				TiXmlElement* hole = new TiXmlElement("Hole");
				hole->SetAttribute("x", x);
				hole->SetAttribute("y", y);
				root->LinkEndChild(hole);
				hole_count++;
			}
			if(level_data.level_data.grid[y][x].mouse)
			{
				TiXmlElement* mouse = new TiXmlElement("Mouse");
				mouse->SetAttribute("x", x);
				mouse->SetAttribute("y", y);
				switch(level_data.level_data.grid[y][x].direction)
				{
				case 0:
					mouse->SetAttribute("d", "East");
					break;
				case 1:
					mouse->SetAttribute("d", "South");
					break;
				case 2:
					mouse->SetAttribute("d", "West");
					break;
				case 3:
					mouse->SetAttribute("d", "North");
					break;
				}
				root->LinkEndChild(mouse);
				mouse_count++;
			}
			if(level_data.level_data.grid[y][x].cat)
			{
				TiXmlElement* cat = new TiXmlElement("Cat");
				cat->SetAttribute("x", x);
				cat->SetAttribute("y", y);
				switch(level_data.level_data.grid[y][x].direction)
				{
				case 0:
					cat->SetAttribute("d", "East");
					break;
				case 1:
					cat->SetAttribute("d", "South");
					break;
				case 2:
					cat->SetAttribute("d", "West");
					break;
				case 3:
					cat->SetAttribute("d", "North");
					break;
				}
				root->LinkEndChild(cat);
				cat_count++;
			}
			if(level_data.level_data.grid[y][x].arrow)
			{
				TiXmlElement* arrow = new TiXmlElement("Arrow");
				arrow->SetAttribute("x", x);
				arrow->SetAttribute("y", y);
				switch(level_data.level_data.grid[y][x].arrow_direction)
				{
				case 0:
					arrow->SetAttribute("d", "East");
					break;
				case 1:
					arrow->SetAttribute("d", "South");
					break;
				case 2:
					arrow->SetAttribute("d", "West");
					break;
				case 3:
					arrow->SetAttribute("d", "North");
					break;
				}
				root->LinkEndChild(arrow);
				arrow_count++;
			}

			if(level_data.level_data.grid[y][x].unknown10 || 
			   level_data.level_data.grid[y][x].unknown11 ||
			   level_data.level_data.grid[y][x].unknown5)
			{
				unknown_count++;
			}
		}
	}

	doc.SaveFile();
	if(mouse_count == 0 || rocket_count == 0)
	{
		std::cout << "Warning, not enough rockets or mice: Mice:"<< mouse_count << ", rockets:" << rocket_count << "\n";
	}
	std::cout << "Offset: " << _offset << " Extracted " << level_name << "\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ifstream rom;
	rom.open("Chu Chu Rocket.gba", std::ios::binary);
	if(!rom.good())
	{
		std::cout << "Unable to open file\n";
		return -1;
	}

	std::cout << "Size of level is " << sizeof(Level) << "\n";
	
	/* Sega named levels */
	for(int level = 0; level < 100; level++)
	{
		Level level_data;
		rom.seekg(0xAF3E8 + 296 * level);
		rom.read((char*)&level_data, 296);
		
		dump_level(LevelToUserLevel(level_data), "World " + lexical_cast<std::string, int>((level / 25) + 1) + 
			" Level " + boost::str(boost::format("%02d") % ((level % 25) + 1)) + ".Level",
			 lexical_cast<std::string, int>(0xAF3E8 + 296 * level),
			 "Sega");
	}

	/* User levels */
	for(int offset = 0; offset < 1461; offset++)
	{
		UserLevel level_data;
		rom.seekg(0xf7190 + offset * 256);
		rom.read((char*)&level_data, 256);

		dump_level(level_data, "User level " + boost::str(boost::format("%04d") % (offset + 1)) + ".Level",
			 lexical_cast<std::string, int>(0xf7190 + offset * 256),
			 "User level");
	}

	/* Mystery levels */
	for(int level = 0; level < 1029; level++)
	{
		UserLevel level_data;
		rom.seekg(0xb6790 + level * 256);
		rom.read((char*)&level_data, 256);

		dump_level(level_data, "Unnamed level " + boost::str(boost::format("%04d") % (level + 1)) + ".level",
			 lexical_cast<std::string, int>(0xb6790 + level * 256),
			 "User level");
	}


	rom.close();
	return 0;
}

