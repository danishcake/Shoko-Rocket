#include "stdafx.h"
#include <MPWorld.h>
#include <Walker.h>

TEST(MPWorldBasics)
{
	MPWorld* pWorld = new MPWorld();
	CHECK(pWorld);
	delete pWorld;
}

TEST(MPWorldManualSpawners)
{
	MPWorld* pWorld = new MPWorld();

	pWorld->SetSpawner(Vector2i(0,0), Direction::East);
	CHECK_EQUAL(Direction::East, pWorld->GetSpawners().at(0).direction);
	CHECK_EQUAL(Vector2i(0, 0), pWorld->GetSpawners().at(0).position);
	pWorld->SetSpawner(Vector2i(0,0), Direction::South);
	CHECK_EQUAL(Direction::South, pWorld->GetSpawners().at(0).direction);
	pWorld->SetSpawner(Vector2i(0,0), Direction::Stopped);
	CHECK_EQUAL(0, pWorld->GetSpawners().size());

	//Now test togglers
	pWorld->ToggleSpawner(Vector2i(1,1), Direction::West);
	CHECK_EQUAL(Direction::West, pWorld->GetSpawners().at(0).direction);
	CHECK_EQUAL(1, pWorld->GetSpawners().size());
	pWorld->ToggleSpawner(Vector2i(1,1), Direction::West);
	CHECK_EQUAL(0, pWorld->GetSpawners().size());
	pWorld->ToggleSpawner(Vector2i(1,1), Direction::South);
	CHECK_EQUAL(Vector2i(1, 1), pWorld->GetSpawners().at(0).position);
	CHECK_EQUAL(Direction::South, pWorld->GetSpawners().at(0).direction);
	CHECK_EQUAL(1, pWorld->GetSpawners().size());
	pWorld->ToggleSpawner(Vector2i(1,1), Direction::North);
	CHECK_EQUAL(Vector2i(1, 1), pWorld->GetSpawners().at(0).position);
	CHECK_EQUAL(Direction::North, pWorld->GetSpawners().at(0).direction);
	CHECK_EQUAL(1, pWorld->GetSpawners().size());

	delete pWorld;
}

TEST(MPWorldManualPlayerRockets)
{
	MPWorld* pWorld = new MPWorld();

	pWorld->SetPlayerRocket(Vector2i(0,0), 0);
	CHECK_EQUAL(1, pWorld->GetPlayerRockets().size());
	pWorld->SetPlayerRocket(Vector2i(0,4), 4);
	CHECK_EQUAL(2, pWorld->GetPlayerRockets().size());

	delete pWorld;
}

TEST(MPWorldLoadsSpawners)
{
	MPWorld* pWorld = new MPWorld("SpawnersLoad.Level");
	CHECK(!pWorld->GetError());

	CHECK_EQUAL(2, pWorld->GetSpawners().size());
	CHECK_EQUAL(Vector2i(0, 0), pWorld->GetSpawners().at(0).position);


	CHECK_EQUAL(Direction::South, pWorld->GetSpawners().at(1).direction);
	CHECK_EQUAL(Vector2i(5, 5), pWorld->GetSpawners().at(1).position);
	delete pWorld;
}

TEST(MPWorldLoadsPlayerRockets)
{
	MPWorld* pWorld = new MPWorld("PlayerRocketsLoad.Level");
	CHECK(!pWorld->GetError());

	CHECK_EQUAL(2, pWorld->GetPlayerRockets().size());
	CHECK_EQUAL(Vector2i(2,2), pWorld->GetPlayerRockets().at(0).position);
	CHECK_EQUAL(Vector2i(5,5), pWorld->GetPlayerRockets().at(1).position);
	CHECK_EQUAL(1, pWorld->GetPlayerRockets().at(0).player_id);
	CHECK_EQUAL(0, pWorld->GetPlayerRockets().at(1).player_id);



	delete pWorld;
}

TEST(MPWorldRandomisePlayerRockets)
{
}

TEST(MPWorldSetPlayerArrows)
{
	MPWorld* pWorld = new MPWorld();
	pWorld->SetPlayerArrow(Vector2i(1, 1), Direction::East,  0, PlayerArrowLevel::FullArrow);
	CHECK_EQUAL(Direction::East, pWorld->GetPlayerArrows().at(0).direction);
	CHECK_EQUAL(false, pWorld->GetPlayerArrows().at(0).halved);
	CHECK_EQUAL(Vector2i(1, 1), pWorld->GetPlayerArrows().at(0).position);
	CHECK_EQUAL(0, pWorld->GetPlayerArrows().at(0).player_id);
	
	pWorld->SetPlayerArrow(Vector2i(1, 1), Direction::East,  0, PlayerArrowLevel::HalfArrow);
	CHECK_EQUAL(Direction::East, pWorld->GetPlayerArrows().at(0).direction);
	CHECK_EQUAL(true, pWorld->GetPlayerArrows().at(0).halved);
	CHECK_EQUAL(Vector2i(1, 1), pWorld->GetPlayerArrows().at(0).position);
	CHECK_EQUAL(0, pWorld->GetPlayerArrows().at(0).player_id);

	pWorld->SetPlayerArrow(Vector2i(1, 1), Direction::West,  0, PlayerArrowLevel::HalfArrow);
	CHECK_EQUAL(Direction::West, pWorld->GetPlayerArrows().at(0).direction);
	CHECK_EQUAL(true, pWorld->GetPlayerArrows().at(0).halved);
	CHECK_EQUAL(Vector2i(1, 1), pWorld->GetPlayerArrows().at(0).position);
	CHECK_EQUAL(0, pWorld->GetPlayerArrows().at(0).player_id);

	pWorld->SetPlayerArrow(Vector2i(3, 3), Direction::South,  2, PlayerArrowLevel::HalfArrow);
	CHECK_EQUAL(2, pWorld->GetPlayerArrows().size());
	CHECK_EQUAL(Direction::South, pWorld->GetPlayerArrows().at(1).direction);
	CHECK_EQUAL(true, pWorld->GetPlayerArrows().at(1).halved);
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetPlayerArrows().at(1).position);
	CHECK_EQUAL(2, pWorld->GetPlayerArrows().at(1).player_id);

	pWorld->SetPlayerArrow(Vector2i(3, 3), Direction::South,  2, PlayerArrowLevel::Clear);
	CHECK_EQUAL(1, pWorld->GetPlayerArrows().size());

	delete pWorld;
}

TEST(MPWorldTickMovesMiceAndCats)
{
	MPWorld* pWorld = new MPWorld();
	Walker* pCat = new Walker();
	pCat->SetPosition(Vector2f(0,0));
	pCat->SetDirection(Direction::East);

	Walker* pMouse = new Walker();
	pMouse->SetPosition(Vector2f(0, 10));
	pMouse->SetDirection(Direction::East);

	pWorld->AddCat(pCat);
	pWorld->AddMouse(pMouse);
	pWorld->Tick(1);
	CHECK_CLOSE(2.0/3.0, pCat->GetPosition().x, 0.05);
	CHECK_CLOSE(1.0, pMouse->GetPosition().x, 0.05);

	delete pWorld;
}

TEST(MPWorldPlayerArrowsNoEffect)
{
	MPWorld* pWorld = new MPWorld();
	pWorld->AddCat(Vector2i(0,0), Direction::East);
	pWorld->SetPlayerArrow(Vector2i(3, 0), Direction::South, 0, PlayerArrowLevel::FullArrow);
	pWorld->Tick(6);
	CHECK_EQUAL(Direction::East, pWorld->GetCats().at(0)->GetDirection());

	delete pWorld;
}

TEST(MPWorldRocketsHolesNoEffect)
{
	MPWorld* pWorld = new MPWorld();
	pWorld->AddCat(Vector2i(0,0), Direction::East);
	pWorld->AddCat(Vector2i(0,1), Direction::East);
	pWorld->ToggleHole(Vector2i(1,0));
	pWorld->ToggleRocket(Vector2i(1,1));
	WorldState::Enum ws = pWorld->Tick(3);
	CHECK_EQUAL(2, pWorld->GetCats().size());
	CHECK_EQUAL(0, pWorld->GetDeadCats().size());
	CHECK_EQUAL(WorldState::OK, ws);

	delete pWorld;
}

TEST(MPWorldCatPositionPatchable)
{
	/* Entities advance blindly ignoring arrows and holes etc. They only turn when they receive 
	   an instruction to do so. This may be backdated so as to get the position right 
	   Units of time are milliseconds, counting from game start */
	MPWorld* pWorld = new MPWorld();
	Walker* pCat = new Walker();
	pCat->SetPosition(Vector2i(0, 0));
	pCat->SetDirection(Direction::South);
	pCat->SetID(10);
	pWorld->AddCat(pCat);

	pWorld->Tick(3);
	CHECK_EQUAL(3000, pWorld->GetTime());
	CHECK_CLOSE(2.0, pCat->GetPosition().y, 0.05);

	pWorld->UpdateCat(pCat->GetID(), Vector2f(0, 1), Direction::East, 1500);
	CHECK_CLOSE(1.0, pCat->GetPosition().y, 0.05);
	CHECK_CLOSE(1.0, pCat->GetPosition().x, 0.05);

	delete pWorld;
}

TEST(MPWorldMousePositionPatchable)
{
	MPWorld* pWorld = new MPWorld();
	Walker* pMouse = new Walker();
	pMouse->SetPosition(Vector2i(0, 0));
	pMouse->SetDirection(Direction::South);
	pMouse->SetID(10);
	pWorld->AddMouse(pMouse);

	pWorld->Tick(3);
	CHECK_EQUAL(3000, pWorld->GetTime());
	CHECK_CLOSE(3.0, pMouse->GetPosition().y, 0.05);

	pWorld->UpdateMouse(pMouse->GetID(), Vector2f(0, 1), Direction::East, 1000);
	CHECK_CLOSE(1.0, pMouse->GetPosition().y, 0.05);
	CHECK_CLOSE(2.0, pMouse->GetPosition().x, 0.05);

	delete pWorld;
}

TEST(MPWorldKillMouse)
{
	//Kill a mouse at a location & time. No need to find current position as purely asthetic
	MPWorld* pWorld = new MPWorld();
	Walker* pMouse = new Walker();
	pMouse->SetPosition(Vector2f(1, 1));
	pMouse->SetDirection(Direction::East);
	pMouse->SetID(100);
	pWorld->AddMouse(pMouse);

	pWorld->Tick(1);
	CHECK_CLOSE(2.0, pMouse->GetPosition().x, 0.05);

	pWorld->KillMouse(100, Vector2f(0.5, 0), 500); //Mouse goes to just dead mice
	CHECK_CLOSE(0.5, pMouse->GetPosition().x, 0.05);

	delete pWorld;
}
TEST(MPWorldCreateMouse)
{
	//Create a mouse at a location & time, find current position
	MPWorld* pWorld = new MPWorld();
	pWorld->CreateMouse(5, Vector2f(1, 1), Direction::East, 500);
	CHECK_EQUAL(500, pWorld->GetTime());
	CHECK_CLOSE(1.0, pWorld->GetMice().at(0)->GetPosition().x, 0.05);
	pWorld->Tick(1);
	CHECK_EQUAL(1500, pWorld->GetTime());
	pWorld->CreateMouse(6, Vector2f(1, 1), Direction::East, 1000);
	CHECK_CLOSE(1.5, pWorld->GetMice().at(1)->GetPosition().x, 0.05);

	delete pWorld;
}
TEST(MPWorldKillCat)
{
	//Kill a cat at a location & time. No need to find current position as purely asthetic
	MPWorld* pWorld = new MPWorld();
	Walker* pCat = new Walker();
	pCat->SetPosition(Vector2f(1, 1));
	pCat->SetDirection(Direction::East);
	pCat->SetID(100);
	pWorld->AddCat(pCat);

	pWorld->Tick(1);
	CHECK_CLOSE(5.0 / 3.0, pCat->GetPosition().x, 0.05);

	pWorld->KillCat(100, Vector2f(0.5, 0), 500);
	CHECK_CLOSE(0.5, pCat->GetPosition().x, 0.05);

	delete pWorld;
}
TEST(MPWorldCreateCat)
{
	//Create a cat at a location & time, find current position
	MPWorld* pWorld = new MPWorld();
	pWorld->CreateCat(5, Vector2f(1, 1), Direction::East, 500);
	CHECK_EQUAL(500, pWorld->GetTime());
	CHECK_CLOSE(1.0, pWorld->GetCats().at(0)->GetPosition().x, 0.05);
	CHECK_EQUAL(5, pWorld->GetCats().at(0)->GetID());
	pWorld->Tick(1);
	CHECK_EQUAL(1500, pWorld->GetTime());
	pWorld->CreateCat(6, Vector2f(1, 1), Direction::East, 1000);
	CHECK_CLOSE(1.333, pWorld->GetCats().at(1)->GetPosition().x, 0.05);

	delete pWorld;
}

//Possibility of server running faster than client, so have to fast forward if detected
TEST(UpdateFromFutureCausesFastForward)
{
	MPWorld* pWorld = new MPWorld();
	Walker* pCat = new Walker();
	pCat->SetID(100);
	pCat->SetPosition(Vector2f(0,0));
	pCat->SetDirection(Direction::East);
	pWorld->AddCat(pCat);

	Walker* pCat2 = new Walker();
	pCat2->SetID(101);
	pCat2->SetPosition(Vector2f(0,0));
	pCat2->SetDirection(Direction::South);
	pWorld->AddCat(pCat2);

	pWorld->Tick(1);
	CHECK_EQUAL(1000, pWorld->GetTime());
	CHECK_CLOSE(2.0/3.0, pCat->GetPosition().x, 0.05);
	CHECK_CLOSE(2.0/3.0, pCat2->GetPosition().y, 0.05);

	pWorld->UpdateCat(100, Vector2f(0,0), Direction::East, 1500);
	CHECK_EQUAL(1500, pWorld->GetTime());
	CHECK_CLOSE(0, pCat->GetPosition().x, 0.05);
	CHECK_CLOSE(1.0, pCat2->GetPosition().y, 0.05);
}

//Possibility of client running faster than server. In this case pause client until updates catch up
TEST(ClientPausesWhenTooFarAhead)
{
}