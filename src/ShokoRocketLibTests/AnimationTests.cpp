#include "stdafx.h"
#include "TextureManager.h"
#include "Animation.h"
#include "AnimationFrame.h"
#include "AnimationSet.h"

TEST(FrameSelecting)
{
	TextureManager::SetTextureManager(new TextureManager());
	Animation a;
	a.AddFrame(new AnimationFrame(1, 0.5f, Vector2i(0,0)));
	a.AddFrame(new AnimationFrame(2, 0.5f, Vector2i(0,0)));
	a.AddFrame(new AnimationFrame(3, 0.5f, Vector2i(0,0)));
	a.AddFrame(new AnimationFrame(4, 0.5f, Vector2i(0,0)));

	CHECK_EQUAL(1, a.GetFrame(0.25f)->GetFrameID());
	CHECK_EQUAL(2, a.GetFrame(0.75f)->GetFrameID());
	CHECK_EQUAL(3, a.GetFrame(1.25f)->GetFrameID());
	CHECK_EQUAL(4, a.GetFrame(1.75f)->GetFrameID());
	//Check wrapping
	CHECK_EQUAL(1, a.GetFrameID(2.25f));
	CHECK_EQUAL(2, a.GetFrameID(2.75f));
	CHECK_EQUAL(3, a.GetFrameID(3.25f));
	CHECK_EQUAL(4, a.GetFrameID(3.75f));

	a.Tick(0.6f);
	CHECK_EQUAL(2, a.GetCurrentFrame()->GetFrameID());
	a.Tick(0.5f);
	CHECK_EQUAL(3, a.GetCurrentFrame()->GetFrameID());
	a.Tick(0.5f);
	CHECK_EQUAL(4, a.GetCurrentFrame()->GetFrameID());
	a.Tick(0.5f);
	CHECK_EQUAL(1, a.GetCurrentFrame()->GetFrameID());

	CHECK_EQUAL(1, a.GetFrameByIndex(0)->GetFrameID());
	CHECK_EQUAL(2, a.GetFrameByIndex(1)->GetFrameID());
	CHECK_EQUAL(3, a.GetFrameByIndex(2)->GetFrameID());
	CHECK_EQUAL(4, a.GetFrameByIndex(3)->GetFrameID());
}

TEST(LoadFromFile)
{
	Animation* p_animation = TextureManager::GetAnimation("BasicAnimation.Animation");
	CHECK(p_animation);
	CHECK_EQUAL(p_animation->GetFrame(0.4f), p_animation->GetFrame(2.4f));
	CHECK(p_animation->GetFrame(0.4f) != p_animation->GetFrame(0.8f));
	CHECK(p_animation->GetFrame(0.4f) != p_animation->GetFrame(1.2f));
	CHECK(p_animation->GetFrame(0.4f) != p_animation->GetFrame(1.6f));
	CHECK(p_animation->GetFrame(0.8f) != p_animation->GetFrame(1.2f));
	CHECK(p_animation->GetFrame(0.8f) != p_animation->GetFrame(1.6f));
	CHECK(p_animation->GetFrame(1.2f) != p_animation->GetFrame(1.6f));
}

TEST(AnimationSetBasics)
{
	AnimationSet animation_set;

	Animation* p_an1 = new Animation();
	p_an1->SetName("An1");
	Animation* p_an2 = new Animation();
	p_an2->SetName("An2");

	animation_set.AddAnimation(p_an1);
	animation_set.AddAnimation(p_an2);

	CHECK_EQUAL(2, animation_set.GetAnimationCount());
	CHECK(animation_set.GetAnimation("An1"));
	CHECK(animation_set.GetAnimation("An2"));
	CHECK_EQUAL((Animation*)NULL, animation_set.GetAnimation("An3"));
	CHECK_EQUAL(animation_set.GetAnimation("An1"), animation_set.GetDefaultAnimation());
}

TEST(AnimationSetFromFile)
{
	AnimationSet* p_animation_set = TextureManager::GetAnimationSet("Directions.Animation");
	CHECK(p_animation_set);
	CHECK_EQUAL(4, p_animation_set->GetAnimationCount());
	CHECK(p_animation_set->GetAnimation("North"));
	CHECK(p_animation_set->GetAnimation("South"));
	CHECK(p_animation_set->GetAnimation("East"));
	CHECK(p_animation_set->GetAnimation("West"));
}

TEST(FrameOffsets)
{
	AnimationSet* p_animation_set = TextureManager::GetAnimationSet("Offsets.Animation");
	CHECK(p_animation_set);
	Animation* p_animation = p_animation_set->GetAnimation("North");
	Vector2i offset = p_animation->GetCurrentFrame()->GetOffset();
	CHECK_EQUAL(10, offset.x);
	CHECK_EQUAL(15, offset.y);

}