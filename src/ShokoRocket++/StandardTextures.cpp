#include "StandardTextures.h"
#include <AnimationSet.h>
#include "SDLTextureManager.h"
#include <Direction.h>
#include <Logger.h>
#include <Animation.h>
#include <SDL.h>
#include "Settings.h"
#include "SDLAnimationFrame.h"
#include <boost/lexical_cast.hpp>

namespace StandardTextures
{
	Animation* mouse_animations[5] = {0, 0, 0, 0, 0}; //Each direction
	Animation* mouse_death = NULL;
	Animation* mouse_rescue = NULL;
	Animation* cat_animations[5] = {0, 0, 0, 0, 0}; //Each direction
	Animation* cat_death = NULL;
	Animation* arrows[5] = {0, 0, 0, 0, 0};
	Animation* half_arrows[5] = {0, 0, 0, 0, 0};
	Animation* arrow_sets[5] = {0, 0, 0, 0, 0};
	Animation* arrows_area = 0;
	Animation* rocket_normal_animation = 0;
	Animation* rocket_blastoff_animation = 0;
	Animation* hole_animation = 0;
	Animation* ring_animation = 0;
	Animation* grid_animation = 0;
	Animation* win_animation = 0;
	Animation* fail_animation = 0;
	Animation* exclamation_animation = 0;

	Animation* mp_arrows[8][5];
	Animation* mp_half_arrows[8][5];
	Animation* mp_rockets[8];
	Animation* mp_spawner;

	void LoadTextures()
	{
		//Free textures if already loaded TODO
		//Acquire textures

		AnimationSet* mouse_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetMouseSprite());
		if(mouse_animation_set)
		{
			for(int i = 0; i < 5; i++)
			{
				mouse_animations[i] = mouse_animation_set->GetAnimation(Direction::ToString((Direction::Enum)i));
				if(!mouse_animations[i])
				{
					Logger::ErrorOut() << "Unable to find direction " << Direction::ToString((Direction::Enum)i) << " in Mouse.animation\n";
				}
			}
			mouse_death = mouse_animation_set->GetAnimation("Death");
			if(!mouse_death)
				Logger::ErrorOut() << "Unable to find Mouse death animation\n";
			mouse_rescue = mouse_animation_set->GetAnimation("Rescue");
			if(!mouse_rescue)
				Logger::ErrorOut() << "Unable to find Mouse rescue animation\n";
		} else
		{
			Logger::ErrorOut() << "Unable to load Mouse animations\n";
		}

		AnimationSet* cat_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetCatSprite());
		if(cat_animation_set)
		{
			for(int i = 0; i < 5; i++)
			{
				cat_animations[i] = cat_animation_set->GetAnimation(Direction::ToString((Direction::Enum)i));
				if(!cat_animations[i])
				{
					Logger::ErrorOut() << "Unable to find direction " << Direction::ToString((Direction::Enum)i) << " in Cat.animation\n";
				}
			}
			cat_death = cat_animation_set->GetAnimation("Death");
			if(!cat_death)
				Logger::ErrorOut() << "Unable to find Cat death animation\n";
		} else
		{
			Logger::ErrorOut() << "Unable to load Cat animations\n";
		}

		AnimationSet* arrow_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetArrowsSprite());
		if(arrow_animation_set)
		{
			for(int i = 0; i < 5; i++)
			{
				arrows[i] = arrow_animation_set->GetAnimation(Direction::ToString((Direction::Enum)i));
				if(!arrows[i])
				{
					Logger::ErrorOut() << "Unable to find direction "  << Direction::ToString((Direction::Enum)i) << " in Arrows.animation\n";
				}
			}
		} else
		{
			Logger::ErrorOut() << "Unable to load Arrow animations\n";
		}

		AnimationSet* half_arrow_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetHalfArrowsSprite());
		if(half_arrow_animation_set)
		{
			for(int i = 0; i < 5; i++)
			{
				half_arrows[i] = half_arrow_animation_set->GetAnimation(Direction::ToString((Direction::Enum)i));
				if(!half_arrows[i])
				{
					Logger::ErrorOut() << "Unable to find direction "  << Direction::ToString((Direction::Enum)i) << " in HalfArrows.animation\n";
				}
			}
		} else
		{
			Logger::ErrorOut() << "Unable to load Half Arrow animations\n";
		}

		AnimationSet* arrow_sets_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetArrowSets());
		if(arrow_sets_animation_set)
		{
			for(int i = 0; i < 5; i++)
			{
				arrow_sets[i] = arrow_sets_animation_set->GetAnimation(Direction::ToString((Direction::Enum)i));
				if(!half_arrows[i])
				{
					Logger::ErrorOut() << "Unable to find direction "  << Direction::ToString((Direction::Enum)i) << " in ArrowSets.animation\n";
				}
			}
		} else
		{
			Logger::ErrorOut() << "Unable to load Arrow Sets animations\n";
		}

		mp_spawner = SDLTextureManager::GetAnimation("Spawner.animation");
		hole_animation = SDLTextureManager::GetAnimation(Settings::GetHoleSprite());
		ring_animation = SDLTextureManager::GetAnimation(Settings::GetRingSprite());
		arrows_area = SDLTextureManager::GetAnimation("ArrowsArea.animation");
		AnimationSet* rocket_animation_set = SDLTextureManager::GetAnimationSet(Settings::GetRocketSprite());
		rocket_normal_animation = rocket_animation_set->GetAnimation("Normal");
		win_animation = SDLTextureManager::GetAnimation("WinAnimation.animation");
		fail_animation = SDLTextureManager::GetAnimation("FailAnimation.animation");
		exclamation_animation = SDLTextureManager::GetAnimation("Exclamation.animation");

		if(!hole_animation)
		{
			Logger::ErrorOut() << "Unable to load hole animation\n";
		}

		if(!ring_animation)
		{
			Logger::ErrorOut() << "Unable to load ring animation\n";
		}

		if(!rocket_normal_animation)
		{
			Logger::ErrorOut() << "Unable to load rocket animation\n";
		}

		if(!arrows_area)
		{
			Logger::ErrorOut() << "Unable to load arrow area animation\n";
		}

		if(!win_animation)
		{
			Logger::ErrorOut() << "Unable to load win animation\n";
		}

		if(!fail_animation)
		{
			Logger::ErrorOut() << "Unable to load win animation\n";
		}
		if(!exclamation_animation)
		{
			Logger::ErrorOut() << "Unable to load exclamation animation\n";
		}
		if(!mp_spawner)
		{
			Logger::ErrorOut() << "Unable to load exclamation animation\n";
		}

		AnimationSet* mp_arrow_set = SDLTextureManager::GetAnimationSet("MPArrows.animation");
		if(mp_arrow_set)
		{
			for(int i = 0; i < 8; i++)
			{
				mp_arrows[i][Direction::North] = mp_arrow_set->GetAnimation("NF" + boost::lexical_cast<std::string, int>(i));
				mp_arrows[i][Direction::South] = mp_arrow_set->GetAnimation("SF" + boost::lexical_cast<std::string, int>(i));
				mp_arrows[i][Direction::East] = mp_arrow_set->GetAnimation("EF" + boost::lexical_cast<std::string, int>(i));
				mp_arrows[i][Direction::West] = mp_arrow_set->GetAnimation("WF" + boost::lexical_cast<std::string, int>(i));
				mp_arrows[i][Direction::Stopped] = mp_arrow_set->GetAnimation("Stopped");

				mp_half_arrows[i][Direction::North] = mp_arrow_set->GetAnimation("NH" + boost::lexical_cast<std::string, int>(i));
				mp_half_arrows[i][Direction::South] = mp_arrow_set->GetAnimation("SH" + boost::lexical_cast<std::string, int>(i));
				mp_half_arrows[i][Direction::East] = mp_arrow_set->GetAnimation("EH" + boost::lexical_cast<std::string, int>(i));
				mp_half_arrows[i][Direction::West] = mp_arrow_set->GetAnimation("WH" + boost::lexical_cast<std::string, int>(i));
				mp_half_arrows[i][Direction::Stopped] = mp_arrow_set->GetAnimation("Stopped");
				//TODO error logging
			}
		} else
		{
			Logger::ErrorOut() << "Unable to load MPArrows animations\n";
		}

		AnimationSet* mp_rocket_set = SDLTextureManager::GetAnimationSet("MPRockets.animation");
		if(mp_rocket_set)
		{
			for(int i = 0; i < 8; i++)
			{
				mp_rockets[i] = mp_rocket_set->GetAnimation("Rocket" + boost::lexical_cast<std::string, int>(i));
				if(!mp_rockets[i])
					Logger::ErrorOut() << "Unable to load MPRocket " << i << "\n";
			}
		} else
		{
			Logger::ErrorOut() << "Unable to load MPRockets animations\n";
		}

		//TODO blastoff
	}

	void TickAnimations(float _dt)
	{
		for(int i = 0; i < 5; i++)
		{
			cat_animations[i]->Tick(_dt);
			mouse_animations[i]->Tick(_dt);
		}
		hole_animation->Tick(_dt);
		rocket_normal_animation->Tick(_dt);
		mouse_death->Tick(_dt);
		
	}
}