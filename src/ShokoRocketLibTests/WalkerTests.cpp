#include "stdafx.h"
#include <Walker.h>
#include <SPWorld.h>

TEST(CheckWalkerDirections)
{
	//This should check the walker coordinate system as defined in Direction.h
	Walker* pWalker = new Walker();
	pWalker->SetPosition(Vector2f(5,5));
	pWalker->SetDirection(Direction::North);
	pWalker->SetWalkerType(WalkerType::Mouse);
	pWalker->Advance(1.0f);

	CHECK_CLOSE(5, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(4, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::West);
	pWalker->Advance(1.0f);

	CHECK_CLOSE(4, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(4, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::South);
	pWalker->Advance(1.0f);

	CHECK_CLOSE(4, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(5, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::East);
	pWalker->Advance(1.0f);

	CHECK_CLOSE(5, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(5, pWalker->GetPosition().y, 0.0001);

	
	//Now check time and speed are taken into account - 2/3 speed and 3 time = 2x movement
	pWalker->SetDirection(Direction::North);
	pWalker->SetWalkerType(WalkerType::Cat);
	pWalker->Advance(3);

	CHECK_CLOSE(5, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(3, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::West);
	pWalker->Advance(3);

	CHECK_CLOSE(3, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(3, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::South);
	pWalker->Advance(3);

	CHECK_CLOSE(3, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(5, pWalker->GetPosition().y, 0.0001);

	pWalker->SetDirection(Direction::East);
	pWalker->Advance(3);

	CHECK_CLOSE(5, pWalker->GetPosition().x, 0.0001);
	CHECK_CLOSE(5, pWalker->GetPosition().y, 0.0001);



	delete pWalker;
}

TEST(CheckWalkerTurns)
{
	Walker* pWalker = new Walker();
	SPWorld* pWorld = new SPWorld();
	CHECK_EQUAL((SPWorld*)NULL, pWalker->GetWorld());
	pWalker->SetWorld(pWorld);
	CHECK_EQUAL(pWorld, pWalker->GetWorld());
	
	pWalker->SetPosition(Vector2f(0,0));
	pWalker->SetDirection(Direction::East);
	pWalker->SetWalkerType(WalkerType::Mouse);
	pWalker->Advance(21); // I should turn south during this move
	CHECK_EQUAL(Direction::South, pWalker->GetDirection());
	CHECK_EQUAL(Vector2f(19, 2), pWalker->GetPosition());

	for(int i = 0; i < 100; i++)
		pWalker->Advance(1.0f / 100.0f);
	CHECK_EQUAL(Direction::South, pWalker->GetDirection());
	CHECK_EQUAL(Vector2f(19, 3), pWalker->GetPosition());
	for(int i = 0; i < 100; i++)
		pWalker->Advance(1.0f / 3.0f);
	//Should travel 33 units forward, so heading west
	CHECK_EQUAL(Direction::West, pWalker->GetDirection());
	delete pWalker;
	delete pWorld;
}

TEST(CheckWalkerArrowInteraction)
{
	//Walkers should change following a command from an arrow, but not affect the reset
	Walker* pWalker = new Walker();
	pWalker->SetPosition(Vector2f(2,2));
	pWalker->SetDirection(Direction::South);
	pWalker->Advance(5);
	CHECK_EQUAL(Vector2f(2,7), pWalker->GetPosition());
	CHECK_EQUAL(Direction::South, pWalker->GetDirection());
	
	pWalker->EncounterArrow(Direction::East);
	CHECK_EQUAL(Direction::East, pWalker->GetDirection());

	pWalker->Reset();
	CHECK_EQUAL(Direction::South, pWalker->GetDirection());

	delete pWalker;
}

TEST(CheckWalkerReset)
{
	SPWorld* pWorld = new SPWorld();
	Walker* pWalker = new Walker();
	pWorld->AddMouse(pWalker);
	pWalker->SetPosition(Vector2f(2,2));
	pWalker->SetDirection(Direction::East);
	CHECK_EQUAL(Vector2f(2, 2), pWalker->GetPosition());
	pWalker->Advance(0.5f);
	CHECK_EQUAL(Vector2f(2.5f, 2), pWalker->GetPosition());
	pWalker->Reset();
	CHECK_EQUAL(Vector2f(2, 2), pWalker->GetPosition());

	pWalker->SetDirection(Direction::North);
	pWalker->Advance(5);
	CHECK_EQUAL(Vector2f(5, 0), pWalker->GetPosition());
	CHECK_EQUAL(Direction::East, pWalker->GetDirection());
	pWalker->Reset();
	CHECK_EQUAL(Vector2f(2, 2), pWalker->GetPosition());
	CHECK_EQUAL(Direction::North, pWalker->GetDirection());
	
	delete pWorld;
}

TEST(WalkersStartTheRightWay)
{
	SPWorld* pWorld = new SPWorld();
	pWorld->SetSize(Vector2i(3,3));
	pWorld->AddMouse(Vector2i(2, 2), Direction::East); //Should start going north automatically
	Walker* pMouse = new Walker();
	pMouse->SetPosition(Vector2i(0, 0));
	pMouse->SetDirection(Direction::West);
	pWorld->AddMouse(pMouse); //Should start going south automatically
	pWorld->Tick(0.5f);
	CHECK_CLOSE(2, pWorld->GetMice().at(0)->GetPosition().x, 0.01f);
	CHECK_CLOSE(1.5f, pWorld->GetMice().at(0)->GetPosition().y, 0.01f);
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());

	CHECK_CLOSE(0, pWorld->GetMice().at(1)->GetPosition().x, 0.01f);
	CHECK_CLOSE(0.5f, pWorld->GetMice().at(1)->GetPosition().y, 0.01f);
	CHECK_EQUAL(Direction::South, pWorld->GetMice().at(1)->GetDirection());
	delete pWorld;
}

TEST(WalkerDeathTimer)
{
	Walker* pWalker = new Walker();
	CHECK_EQUAL(0, pWalker->GetDeathTime());
	pWalker->DeathTick(0.5f);
	CHECK_CLOSE(0.5f, pWalker->GetDeathTime(), 0.001f);

	delete pWalker;
}