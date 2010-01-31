#include "TextureManager.h"
#include "Animation.h"
#include "AnimationFrame.h"
#include "AnimationSet.h"
#include <TinyXML.h>
#include "Logger.h"

using std::string;
using std::map;

TextureManager* TextureManager::instance_ = NULL;

TextureManager* TextureManager::GetInstance()
{
	if(instance_ == NULL)
		instance_ = new TextureManager();
	return instance_;
}

AnimationSet* TextureManager::AddAnimationSet(string _xml_animation_set)
{
	AnimationSet* p_animation_set = new AnimationSet();
	bool error = false;

	TiXmlDocument animation_set_doc;
	animation_set_doc.LoadFile("Animations/" + _xml_animation_set);
	/* Load the document and interate through each 'Animation' element. 
	 * Within each animation iterator over each frame
	 */
	if(!animation_set_doc.Error())
	{
		TiXmlElement* p_root = animation_set_doc.FirstChildElement("AnimationSet");
		if(p_root)
		{
			TiXmlElement* p_animation_el = p_root->FirstChildElement("Animation");
			int animation_id = 0;
			while(p_animation_el)
			{
				Animation* p_animation = new Animation();
				string name;
				if(p_animation_el->QueryValueAttribute("Name", &name) != TIXML_SUCCESS)
					Logger::DiagnosticOut() << _xml_animation_set << ": Animation ID: " << animation_id << ": Animation not named\n";
				else
					p_animation->SetName(name);

				TiXmlElement* p_frame_el = p_animation_el->FirstChildElement("Frame");
				while(p_frame_el)
				{
					int width = -1;
					int height = -1;
					int left = -1;
					int top = -1;
					float frame_time = -1;
					std::string file;

					if(p_frame_el->QueryIntAttribute("Width", &width) == TIXML_SUCCESS &&
					   p_frame_el->QueryIntAttribute("Height", &height) == TIXML_SUCCESS &&
					   p_frame_el->QueryIntAttribute("Left", &left) == TIXML_SUCCESS &&
					   p_frame_el->QueryIntAttribute("Top", &top) == TIXML_SUCCESS &&
					   p_frame_el->QueryValueAttribute("File", &file) == TIXML_SUCCESS &&
					   p_frame_el->QueryFloatAttribute("Time", &frame_time) == TIXML_SUCCESS)
					{
						//Optional parameters
						Vector2i frame_offset = Vector2i(0,0);
						p_frame_el->QueryIntAttribute("OffsetX", &frame_offset.x);
						p_frame_el->QueryIntAttribute("OffsetY", &frame_offset.y);
						AnimationFrame* p_frame = instance_->AcquireResource(Vector2i(left, top), Vector2i(width, height), file, frame_time, frame_offset);
						p_animation->AddFrame(p_frame);
					} else
					{
						Logger::ErrorOut() << "Error parsing a frame - either Width, height, left, top, file or time is missing\n";
						error = true;
					}

					p_frame_el = p_frame_el->NextSiblingElement("Frame");
				}
				p_animation_el = p_animation_el->NextSiblingElement("Animation");
				p_animation_set->AddAnimation(p_animation);
				animation_id++;
			}
		} else
		{
			Logger::ErrorOut() << "Root 'AnimationSet' element missing\n";
			error = true;
		}
	} else
	{
		Logger::ErrorOut() << "Unable to open file: " << _xml_animation_set << "\n" <<
								animation_set_doc.ErrorDesc() << "\n";
		error = true;
	}

	if(error)
	{
		delete p_animation_set;
		p_animation_set = NULL;
	} else
	{
		GetInstance()->animations_[_xml_animation_set] = p_animation_set;
	}

	return p_animation_set;
}

AnimationSet* TextureManager::GetAnimationSet(string _xml_animation_set)
{
	if(GetInstance()->animations_.find(_xml_animation_set) != GetInstance()->animations_.end())
	{
		return GetInstance()->animations_[_xml_animation_set];
	} else
	{
		return GetInstance()->AddAnimationSet(_xml_animation_set);
	}
}


/* Either loads the animation, or if already loaded returns a copy of it */
/* If there is more than one animation then the first is returned */
Animation* TextureManager::GetAnimation(std::string _xml_animation)
{
	if(GetInstance()->animations_.find(_xml_animation) !=  GetInstance()->animations_.end())
	{
		return GetInstance()->animations_[_xml_animation]->GetDefaultAnimation();
	} else
	{
		AnimationSet* as = GetInstance()->AddAnimationSet(_xml_animation);
		if(as)
			return as->GetDefaultAnimation();
		else
			return NULL;
	}
}


AnimationFrame* TextureManager::AcquireResource(Vector2i _offset, Vector2i _size, string _filename, float _time, Vector2i _frame_offset)
{
	//Nothing done, override in SDL/OpenGL implementations
	return new AnimationFrame(-1, _time, _frame_offset);
}