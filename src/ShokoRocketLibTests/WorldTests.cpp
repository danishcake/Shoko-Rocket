#include "stdafx.h"
#include <World.h>
#include <Walker.h>

TEST(CheckWorldDefaults)
{
	World* pWorld = new World();
	CHECK_EQUAL(pWorld->GetSize(), Vector2i(20,20));

	GridSquare gs_nw = pWorld->GetGridSquare(Vector2i(0,0));
	CHECK_EQUAL(gs_nw.GetNorth(), true);
	CHECK_EQUAL(gs_nw.GetWest(), true);
	CHECK_EQUAL(gs_nw.GetSouth(), false);
	CHECK_EQUAL(gs_nw.GetEast(), false);

	GridSquare gs_ne = pWorld->GetGridSquare(Vector2i(pWorld->GetSize().x-1, 0));
	CHECK_EQUAL(gs_ne.GetNorth(), true);
	CHECK_EQUAL(gs_ne.GetWest(), false);
	CHECK_EQUAL(gs_ne.GetSouth(), false);
	CHECK_EQUAL(gs_ne.GetEast(), true);

	GridSquare gs_sw = pWorld->GetGridSquare(Vector2i(0, pWorld->GetSize().y - 1));
	CHECK_EQUAL(gs_sw.GetNorth(), false);
	CHECK_EQUAL(gs_sw.GetWest(), true);
	CHECK_EQUAL(gs_sw.GetSouth(), true);
	CHECK_EQUAL(gs_sw.GetEast(), false);

	GridSquare gs_se = pWorld->GetGridSquare(Vector2i(pWorld->GetSize().x-1, pWorld->GetSize().y - 1));
	CHECK_EQUAL(gs_se.GetNorth(), false);
	CHECK_EQUAL(gs_se.GetWest(), false);
	CHECK_EQUAL(gs_se.GetSouth(), true);
	CHECK_EQUAL(gs_se.GetEast(), true);

	delete pWorld;
}

TEST(CheckWorldResize)
{
	World* pWorld = new World();
	pWorld->SetSize(Vector2i(10,10));
	CHECK_EQUAL(Vector2i(10,10), pWorld->GetSize());

	GridSquare gs_nw = pWorld->GetGridSquare(Vector2i(0,0));
	CHECK_EQUAL(gs_nw.GetNorth(), true);
	CHECK_EQUAL(gs_nw.GetWest(), true);
	CHECK_EQUAL(gs_nw.GetSouth(), false);
	CHECK_EQUAL(gs_nw.GetEast(), false);

	GridSquare gs_se = pWorld->GetGridSquare(Vector2i(9, 9));
	CHECK_EQUAL(gs_se.GetNorth(), false);
	CHECK_EQUAL(gs_se.GetWest(), false);
	CHECK_EQUAL(gs_se.GetSouth(), true);
	CHECK_EQUAL(gs_se.GetEast(), true);


	delete pWorld;

}

TEST(CheckWorldWallEdit)
{
	World* pWorld = new World();

	GridSquare gs_ne = pWorld->GetGridSquare(Vector2i(0,0));
	CHECK(gs_ne.GetNorth());
	gs_ne.ClearNorth();

	pWorld->SetGridSquare(Vector2i(0,0), gs_ne);

	GridSquare gs_ne2 = pWorld->GetGridSquare(Vector2i(0, 0));
	GridSquare gs_se = pWorld->GetGridSquare(Vector2i(0, pWorld->GetSize().y - 1));
	CHECK(!gs_ne2.GetNorth());
	CHECK(!gs_se.GetSouth());

	delete pWorld;
}

TEST(CheckWorldWallToggle)
{
	World* pWorld = new World();
	//CHECK_EQUAL(false, WalkerAt(Vector2i(0, 0), Direction::West, pWorld->GetCats()));
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(1, 1)).GetWest());
	pWorld->ToggleWall(Vector2i(1, 1), Direction::West);
	CHECK_EQUAL(true, pWorld->GetGridSquare(Vector2i(1, 1)).GetWest());
}

TEST(WorldWithWalls)
{
	World* pWorld = new World("Empty10x10.Level");
	CHECK_EQUAL(Vector2i(10, 10), pWorld->GetSize());
	CHECK_EQUAL("Empty 10x10", pWorld->GetName());
	CHECK_EQUAL(false, pWorld->GetError());
	delete pWorld;

	pWorld = new World("OutsideOnly3x3.Level");
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetSize());
	CHECK_EQUAL("Outside only 3x3", pWorld->GetName());
	CHECK_EQUAL(false, pWorld->GetError());

	CHECK_EQUAL(true, pWorld->GetGridSquare(Vector2i(0,0)).GetNorth());
	CHECK_EQUAL(true, pWorld->GetGridSquare(Vector2i(0,0)).GetWest());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(0,0)).GetSouth());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(0,0)).GetEast());

	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(1,1)).GetNorth());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(1,1)).GetWest());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(1,1)).GetSouth());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(1,1)).GetEast());

	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(2,2)).GetNorth());
	CHECK_EQUAL(false, pWorld->GetGridSquare(Vector2i(2,2)).GetWest());
	CHECK_EQUAL(true, pWorld->GetGridSquare(Vector2i(2,2)).GetSouth());
	CHECK_EQUAL(true, pWorld->GetGridSquare(Vector2i(2,2)).GetEast());
	delete pWorld;
}

TEST(ManuallyAddedMice)
{
	World* pWorld = new World();
	pWorld->SetSize(Vector2i(3,3));
	pWorld->AddMouse(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(1, pWorld->GetMice().size());
	CHECK_EQUAL(Vector2f(0,0), pWorld->GetMice().at(0)->GetPosition());
	Walker* pMouse = new Walker();
	pMouse->SetPosition(Vector2f(2,2));
	pMouse->SetDirection(Direction::East);
	pWorld->AddMouse(pMouse);
	CHECK_EQUAL(2, pWorld->GetMice().size());
	CHECK_EQUAL(Vector2f(0,0), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Vector2f(2,2), pWorld->GetMice().at(1)->GetPosition());

	pWorld->Tick(0.5f); 
	CHECK_CLOSE(0.5f, pWorld->GetMice().at(0)->GetPosition().x, 0.01f);
	CHECK_CLOSE(0.0f, pWorld->GetMice().at(0)->GetPosition().y, 0.01f);

	CHECK_CLOSE(2.0f, pWorld->GetMice().at(1)->GetPosition().x, 0.01f);
	CHECK_CLOSE(1.5f, pWorld->GetMice().at(1)->GetPosition().y, 0.01f);
	delete pWorld;
}

TEST(ManuallyAddedCats)
{
	World* pWorld = new World();
	pWorld->SetSize(Vector2i(3,3));
	pWorld->AddCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(1, pWorld->GetCats().size());
	CHECK_EQUAL(Vector2f(0,0), pWorld->GetCats().at(0)->GetPosition());
	Walker* pCat = new Walker();
	pCat->SetPosition(Vector2f(2,2));
	pCat->SetDirection(Direction::East);
	pWorld->AddCat(pCat);
	CHECK_EQUAL(2, pWorld->GetCats().size());
	CHECK_EQUAL(Vector2f(0,0), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Vector2f(2,2), pWorld->GetCats().at(1)->GetPosition());

	pWorld->Tick(1.5f); 
	CHECK_CLOSE(1.0f, pWorld->GetCats().at(0)->GetPosition().x, 0.01f);
	CHECK_CLOSE(0.0f, pWorld->GetCats().at(0)->GetPosition().y, 0.01f);

	CHECK_CLOSE(2.0f, pWorld->GetCats().at(1)->GetPosition().x, 0.01f);
	CHECK_CLOSE(1.0f, pWorld->GetCats().at(1)->GetPosition().y, 0.01f);
	delete pWorld;
}

bool WalkerAt(Vector2f _position, Direction::Enum _direction, std::vector<Walker*>& _walkers)
{
	for(std::vector<Walker*>::iterator it = _walkers.begin(); it != _walkers.end(); it++)
	{
		if(((*it)->GetPosition() - _position).length() < 0.001f &&
			(*it)->GetDirection() == _direction)
			return true;
	}
	return false;
}

TEST(CheckMouseToggle)
{
	World* pWorld = new World();
	pWorld->AddMouse(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetMice()));

	pWorld->ToggleMouse(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(false, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetMice()));
	pWorld->ToggleMouse(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetMice()));

	pWorld->ToggleMouse(Vector2i(0, 0), Direction::South);
	CHECK_EQUAL(false, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetMice()));
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::South, pWorld->GetMice()));
	delete pWorld;
}

TEST(CheckCatToggle)
{
	World* pWorld = new World();
	pWorld->AddCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));

	pWorld->ToggleCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(false, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));
	pWorld->ToggleCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));

	pWorld->ToggleCat(Vector2i(0, 0), Direction::South);
	CHECK_EQUAL(false, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::South, pWorld->GetCats()));
	delete pWorld;
}

TEST(CatsToggleMiceEtc)
{
	World* pWorld = new World();
	pWorld->AddCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));
	pWorld->ToggleMouse(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetMice()));
	pWorld->ToggleCat(Vector2i(0, 0), Direction::East);
	CHECK_EQUAL(true, WalkerAt(Vector2i(0, 0), Direction::East, pWorld->GetCats()));
	delete pWorld;
}


TEST(WorldReset)
{
	//When walkers are created or repositioned with SetPosition they store that position to be reset to
	//When they die they are moved to a different list so they can be recreated
	World* pWorld = new World();
	pWorld->AddMouse(Vector2i(5,5), Direction::South);


	pWorld->Tick(0.5f);
	CHECK_EQUAL(Vector2f(5, 5.5f), pWorld->GetMice().at(0)->GetPosition());
	pWorld->Reset();
	CHECK_EQUAL(Vector2f(5, 5), pWorld->GetMice().at(0)->GetPosition());
	pWorld->Tick(16);
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());

	delete pWorld;
}

TEST(WorldWithMice)
{
	World* pWorld = new World("OutsideAndMice3x3.Level");
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetSize());
	CHECK_EQUAL("Outside and mice 3x3", pWorld->GetName());
	CHECK_EQUAL(false, pWorld->GetError());
	CHECK_EQUAL(2, pWorld->GetMice().size());
	delete pWorld;
}

TEST(WorldWithCats)
{
	World* pWorld = new World("OutsideAndCats3x3.Level");
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetSize());
	CHECK_EQUAL("Outside and cats 3x3", pWorld->GetName());
	CHECK_EQUAL(false, pWorld->GetError());
	CHECK_EQUAL(2, pWorld->GetCats().size());
	delete pWorld;
}

TEST(WorldWithHolesAndRocketsManual)
{
	World* pWorld = new World();

	pWorld->SetSquareType(Vector2i(0,0), SquareType::Rocket);
	pWorld->SetSquareType(Vector2i(1,0), SquareType::Hole);
	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(0,0)));
	CHECK_EQUAL(SquareType::Hole, pWorld->GetSquareType(Vector2i(1,0)));
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0,1)));
	delete pWorld;
}

TEST(CheckHoleAndRocketToggle)
{
	World* pWorld = new World();
	pWorld->ToggleRocket(Vector2i(0,0));
	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(0,0)));
	pWorld->ToggleRocket(Vector2i(0,0));
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0,0)));

	pWorld->ToggleHole(Vector2i(0,0));
	CHECK_EQUAL(SquareType::Hole, pWorld->GetSquareType(Vector2i(0,0)));
	pWorld->ToggleRocket(Vector2i(0,0));
	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(0,0)));

	delete pWorld;
}

TEST(WorldWithRockets)
{
	World* pWorld = new World("OutsideAndRockets3x3.Level");
	CHECK(!pWorld->GetError());
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetSize());
	CHECK_EQUAL("Outside and rockets 3x3", pWorld->GetName());

	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(0,0)));
	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(2,2)));
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(1,1)));
	delete pWorld;
}

TEST(WorldWithHole)
{
	World* pWorld = new World("OutsideAndHoles3x3.Level");
	CHECK(!pWorld->GetError());
	CHECK_EQUAL(Vector2i(3, 3), pWorld->GetSize());
	CHECK_EQUAL("Outside and holes 3x3", pWorld->GetName());

	CHECK_EQUAL(SquareType::Hole, pWorld->GetSquareType(Vector2i(0,0)));
	CHECK_EQUAL(SquareType::Hole, pWorld->GetSquareType(Vector2i(2,2)));
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(1,1)));
	delete pWorld;
}


TEST(WorldWithAll)
{
	World* pWorld = new World("OutsideWithAll4x4.Level");
	CHECK(!pWorld->GetError());

	CHECK_EQUAL(SquareType::Rocket, pWorld->GetSquareType(Vector2i(0,0)));
	CHECK_EQUAL(SquareType::Hole, pWorld->GetSquareType(Vector2i(1,0)));
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(1,1)));

	CHECK_EQUAL(Vector2i(3,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Vector2i(1,3), pWorld->GetCats().at(0)->GetPosition());

	delete pWorld;
}

TEST(WorldToFile)
{
	World* pWorld = new World("OutsideWithAll4x4.Level");
	pWorld->Tick(0.5f); //Upon save should revert
	pWorld->SaveAs("SaveAsTest.Level");
	delete pWorld;

	World* pWorld2 = new World("SaveAsTest.Level");
	CHECK(!pWorld2->GetError());
	if(!pWorld2->GetError())
	{
		CHECK_EQUAL(1, pWorld2->GetMice().size());
		CHECK_EQUAL(1, pWorld2->GetCats().size());
		CHECK_EQUAL(Vector2f(3,3), pWorld2->GetMice().at(0)->GetPosition());
		CHECK_EQUAL(Direction::West, pWorld2->GetMice().at(0)->GetDirection());
		CHECK_EQUAL(Vector2f(1,3), pWorld2->GetCats().at(0)->GetPosition());
		CHECK_EQUAL(Direction::North, pWorld2->GetCats().at(0)->GetDirection());

		CHECK_EQUAL(SquareType::Rocket, pWorld2->GetSquareType(Vector2i(0,0)));
		CHECK_EQUAL(SquareType::Hole, pWorld2->GetSquareType(Vector2i(1,0)));
	}
	delete pWorld;
}

TEST(WorldStates)
{
	World* pWorld = new World();
	CHECK_EQUAL(WorldState::OK, pWorld->Tick(0));
	delete pWorld;
	pWorld = new World("NotALevel.level");
	CHECK_EQUAL(WorldState::FileLoadError, pWorld->Tick(0));
	delete pWorld;
}

TEST(HolesKillWalkers)
{
	World* pWorld = new World("HolesKillWalkers.Level");
	CHECK(!pWorld->GetError());
	CHECK_EQUAL(1, pWorld->GetMice().size());
	CHECK_EQUAL(1, pWorld->GetCats().size());
	CHECK_EQUAL(0, pWorld->GetDeadMice().size());
	CHECK_EQUAL(0, pWorld->GetDeadCats().size());

	WorldState::Enum state = pWorld->Tick(6); //Should walk far enough to kill the mouse
	CHECK_EQUAL(0, pWorld->GetMice().size());
	CHECK_EQUAL(1, pWorld->GetCats().size());
	CHECK_EQUAL(1, pWorld->GetDeadMice().size());
	CHECK_EQUAL(0, pWorld->GetDeadCats().size());
	CHECK_EQUAL(WorldState::Defeat, state);
	
	pWorld->Reset();

	pWorld->Tick(12 ); //Should walk far enough to kill the cat
	CHECK_EQUAL(0, pWorld->GetMice().size());
	CHECK_EQUAL(0, pWorld->GetCats().size());
	CHECK_EQUAL(1, pWorld->GetDeadMice().size());
	CHECK_EQUAL(1, pWorld->GetDeadCats().size());
	CHECK_EQUAL(WalkerState::Killed, pWorld->GetDeadCats()[0]->GetWalkerState());

	pWorld->Reset();
	CHECK_EQUAL(1, pWorld->GetMice().size());
	CHECK_EQUAL(1, pWorld->GetCats().size());
	CHECK_EQUAL(0, pWorld->GetDeadMice().size());
	CHECK_EQUAL(0, pWorld->GetDeadCats().size());

	delete pWorld;
}

TEST(RocketsRescueMice)
{
	World* pWorld = new World("RocketsRescueMice.Level");
	CHECK(!pWorld->GetError());
	CHECK_EQUAL(3, pWorld->GetMice().size());
	CHECK_EQUAL(3, pWorld->GetTotalMice());
	CHECK_EQUAL(0, pWorld->GetRescuedMice());
	pWorld->Tick(6);
	CHECK_EQUAL(2, pWorld->GetMice().size());
	CHECK_EQUAL(3, pWorld->GetTotalMice());
	CHECK_EQUAL(1, pWorld->GetRescuedMice());
	pWorld->Tick(1);
	CHECK_EQUAL(1, pWorld->GetMice().size());
	CHECK_EQUAL(3, pWorld->GetTotalMice());
	CHECK_EQUAL(2, pWorld->GetRescuedMice());
	pWorld->Tick(1);
	CHECK_EQUAL(0, pWorld->GetMice().size());
	CHECK_EQUAL(3, pWorld->GetRescuedMice());
	CHECK_EQUAL(WorldState::Victory, pWorld->Tick(0));
	delete pWorld;
}

TEST(CatsKillRockets)
{
	World* pWorld = new World("CatsKillRockets.Level");
	CHECK(!pWorld->GetError());
	CHECK_EQUAL(1, pWorld->GetCats().size());
	CHECK_EQUAL(WorldState::Defeat, pWorld->Tick(15));
	delete pWorld;
}

TEST(WorldCleansUpWalkers)
{
	int sumWalkers = Walker::Total_created - Walker::Total_destroyed;
	World* pWorld = new World("OutsideWithAll4x4.Level");
	CHECK(sumWalkers < (Walker::Total_created - Walker::Total_destroyed));
	delete pWorld;
	CHECK(sumWalkers == (Walker::Total_created - Walker::Total_destroyed));
}

TEST(CatMouseHeadToHead)
{
	World* pWorld = new World("CatMouseHeadToHead.Level");
	CHECK_EQUAL(1, pWorld->GetMice().size());
	CHECK_EQUAL(0, pWorld->GetDeadMice().size());
	CHECK_EQUAL(WorldState::OK, pWorld->Tick(0));

	CHECK_EQUAL(WorldState::OK, pWorld->Tick(1));

	CHECK_EQUAL(WorldState::Defeat, pWorld->Tick(0.8f));
	CHECK_EQUAL(0, pWorld->GetMice().size());
	CHECK_EQUAL(1, pWorld->GetDeadMice().size());
	delete pWorld;
}

TEST(CloseCatsMiss)
{//Cat Walks across path of mouse
	World* pWorld = new World("CloseCatMiss.Level");
	WorldState::Enum e;
	float total_time = 0;
	while((e = pWorld->Tick(0.1f)) == WorldState::OK)
	{
		total_time += 0.1f;
	}
	CHECK_EQUAL(WorldState::Victory, e);
	CHECK_CLOSE(6.0f, total_time, 0.01f);
	delete pWorld;
}

TEST(MouseFromBehind)
{ //Mouse walks into the back of a cat from behind
  //Collision distance is 0.1
	World* pWorld = new World("MouseFromBehind.Level");
	WorldState::Enum e;
	float total_time = 0;
	while((e = pWorld->Tick(0.01f)) == WorldState::OK)
	{
		total_time += 0.01f;
	}
	CHECK_EQUAL(WorldState::Defeat, e);
	CHECK_CLOSE(8.7f, total_time, 0.01f);
	delete pWorld;
}

TEST(CannotPlaceArrowOnRocketOrHole)
{
	World* pWorld = new World("OutsideWithAll4x4.Level");
	CHECK_EQUAL(false, pWorld->SetSquareType(Vector2i(0,0), SquareType::NorthArrow));
	CHECK_EQUAL(false, pWorld->SetSquareType(Vector2i(1,0), SquareType::SouthArrow));
	CHECK_EQUAL(true, pWorld->SetSquareType(Vector2i(2,0), SquareType::EastArrow));
	CHECK_EQUAL(true, pWorld->SetSquareType(Vector2i(2,1), SquareType::WestArrow));
	delete pWorld;
}

TEST(TrappedWalkersStop)
{
	World* pWorld = new World("MouseTrap!.Level");
	pWorld->Tick(0.1f);
	CHECK_EQUAL(Direction::Stopped, pWorld->GetMice().at(0)->GetDirection());
	delete pWorld;
}

TEST(ArrowsAddedManually)
{
	World* pWorld = new World();
	pWorld->AddArrow(Direction::North);
	pWorld->AddArrow(Direction::South);
	vector<Direction::Enum> arrow_stock = pWorld->GetArrows();
	int north_count = 0;
	int south_count = 0;
	int west_count = 0;
	int east_count = 0;
	for(vector<Direction::Enum>::iterator it = arrow_stock.begin(); it != arrow_stock.end(); ++it)
	{
		switch(*it)
		{
		case Direction::North:
			north_count++;
			break;
		case Direction::South:
			south_count++;
			break;
		case Direction::East:
			east_count++;
			break;
		case Direction::West:
			west_count++;
			break;
		}
	}
	CHECK_EQUAL(1, north_count);
	CHECK_EQUAL(1, south_count);
	CHECK_EQUAL(0, east_count);
	CHECK_EQUAL(0, west_count);


	delete pWorld;
}

int countArrowType(vector<Direction::Enum> _arrows, Direction::Enum _direction)
{
	int arrow_count = 0;
	for(vector<Direction::Enum>::iterator it = _arrows.begin(); it != _arrows.end(); ++it)
	{
		if(*it == _direction)
			arrow_count++;
	}
	return arrow_count;
}

TEST(UnlimitedArrowsMode)
{
	World* pWorld = new World();
	pWorld->ToggleArrow(Vector2i(0,0), Direction::West);
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0, 0)));
	
	pWorld->ToggleArrow(Vector2i(0,1), Direction::South);
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0, 1)));
	
	pWorld->ToggleArrow(Vector2i(0,2), Direction::East);
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0, 2)));
	
	pWorld->ToggleArrow(Vector2i(0,3), Direction::North);
	CHECK_EQUAL(SquareType::Empty, pWorld->GetSquareType(Vector2i(0, 3)));

	pWorld->SetArrowStockUnlimited();

	pWorld->ToggleArrow(Vector2i(0,0), Direction::West);
	CHECK_EQUAL(SquareType::WestArrow, pWorld->GetSquareType(Vector2i(0, 0)));

	pWorld->ToggleArrow(Vector2i(0,1), Direction::South);
	CHECK_EQUAL(SquareType::SouthArrow, pWorld->GetSquareType(Vector2i(0, 1)));
	
	pWorld->ToggleArrow(Vector2i(0,2), Direction::East);
	CHECK_EQUAL(SquareType::EastArrow, pWorld->GetSquareType(Vector2i(0, 2)));
	
	pWorld->ToggleArrow(Vector2i(0,3), Direction::North);
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(0, 3)));
	
}

TEST(ArrowsInFile)
{
	World* pWorld = new World("ArrowsInFile.Level");

	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));
	delete pWorld;
}

TEST(ArrowsConsumed)
{
	World* pWorld = new World("ArrowsInFile.Level");

	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleEastArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::East)); //Decremented
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleEastArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East)); //Incremented
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleNorthArrow(Vector2i(1,1));
	CHECK_EQUAL(0, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleNorthArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West)); 
	
	pWorld->ToggleSouthArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleSouthArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleWestArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleWestArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));
	
	//Now overwrite
	
	pWorld->ToggleEastArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::East)); //Decremented
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleNorthArrow(Vector2i(1,1));
	CHECK_EQUAL(0, countArrowType(pWorld->GetArrows(), Direction::North)); //Decremented
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East)); //Incremented
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));

	pWorld->ToggleSouthArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(4, countArrowType(pWorld->GetArrows(), Direction::West));
	
	pWorld->ToggleWestArrow(Vector2i(1,1));
	CHECK_EQUAL(1, countArrowType(pWorld->GetArrows(), Direction::North));
	CHECK_EQUAL(2, countArrowType(pWorld->GetArrows(), Direction::South));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::East));
	CHECK_EQUAL(3, countArrowType(pWorld->GetArrows(), Direction::West));
	
	delete pWorld;
}

TEST(ArrowsDirectMice)
{
	World* pWorld = new World("ArrowsDirectMice.Level");
	pWorld->SetSquareType(Vector2i(2,3), SquareType::NorthArrow);
	pWorld->SetSquareType(Vector2i(2,1), SquareType::EastArrow);
	pWorld->SetSquareType(Vector2i(3,1), SquareType::NorthArrow);
	pWorld->SetSquareType(Vector2i(1,0), SquareType::SouthArrow);
	pWorld->SetSquareType(Vector2i(1,2), SquareType::WestArrow);
	CHECK_EQUAL(Vector2f(3,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,1), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::East, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(3,1), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(3,0), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,0), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,0), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::South, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,1), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::South, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());


	delete pWorld;
}

TEST(ArrowsDirectCats)
{
	World* pWorld = new World("ArrowsDirectCats.Level");
	pWorld->SetSquareType(Vector2i(2,3), SquareType::NorthArrow);
	pWorld->SetSquareType(Vector2i(2,1), SquareType::EastArrow);
	pWorld->SetSquareType(Vector2i(3,1), SquareType::NorthArrow);
	pWorld->SetSquareType(Vector2i(1,0), SquareType::SouthArrow);
	pWorld->SetSquareType(Vector2i(1,2), SquareType::WestArrow);
	CHECK_EQUAL(Vector2f(3,3), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f); 
	CHECK_EQUAL(Vector2f(2,3), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(2,2), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(2,1), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::East, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(3,1), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(3,0), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(2,0), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(1,0), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::South, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(1,1), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::South, pWorld->GetCats().at(0)->GetDirection());
	pWorld->Tick(1.5f);
	CHECK_EQUAL(Vector2f(1,2), pWorld->GetCats().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetCats().at(0)->GetDirection());


	delete pWorld;
}

TEST(ArrowsInteractWithWallsProperly)
{
	World* pWorld = new World("ArrowsInteractWithWalls.Level");
	pWorld->SetSquareType(Vector2i(0,0), SquareType::NorthArrow);
	pWorld->AddMouse(Vector2i(0,0), Direction::South);
	pWorld->Tick(1);
	CHECK_EQUAL(Vector2f(1,0), pWorld->GetMice().at(0)->GetPosition());
	delete pWorld;
}

TEST(DiminishArrow)
{
	SquareType::Enum arrow = SquareType::NorthArrow;
	arrow = SquareType::Diminish(arrow);
	CHECK_EQUAL(SquareType::HalfNorthArrow, arrow);
	arrow = SquareType::Diminish(arrow);
	CHECK_EQUAL(SquareType::DestroyedNorthArrow, arrow);

	arrow = SquareType::SouthArrow;
	arrow = SquareType::Diminish(arrow);
	CHECK_EQUAL(SquareType::HalfSouthArrow, arrow);
	arrow = SquareType::Diminish(arrow);
	CHECK_EQUAL(SquareType::DestroyedSouthArrow, arrow);
}

TEST(CatsDestroyArrows)
{
	World* pWorld = new World("CatsDestroyArrows.Level");
	pWorld->ToggleSouthArrow(Vector2i(1,1));
	CHECK_EQUAL(SquareType::SouthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	pWorld->Tick(2);
	CHECK_EQUAL(SquareType::HalfSouthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	pWorld->Tick(4);
	CHECK_EQUAL(SquareType::DestroyedSouthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	delete pWorld;
}

TEST(ArrowsReset)
{
	World* pWorld = new World("Empty10x10.Level");
	pWorld->AddArrow(Direction::North);
	pWorld->ToggleNorthArrow(Vector2i(1,2));
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(1,2)));
	CHECK_EQUAL(0, pWorld->GetArrows().size());
	pWorld->AddCat(Vector2i(1,1), Direction::South);
	pWorld->AddCat(Vector2i(1,0), Direction::South);
	pWorld->Tick(2);
	CHECK_EQUAL(SquareType::HalfNorthArrow, pWorld->GetSquareType(Vector2i(1,2)));
	pWorld->Reset();
	//Partially destroyed arrows are restored
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(1,2))); 
	//Fully destroyed arrows are restored
	pWorld->Tick(4);
	CHECK_EQUAL(SquareType::DestroyedNorthArrow, pWorld->GetSquareType(Vector2i(1,2))); 
	pWorld->Reset();
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(1,2))); 
	delete pWorld;

}

TEST(ArrowsClear)
{
	World* pWorld = new World("Empty10x10.Level");
	pWorld->AddArrow(Direction::North);
	pWorld->ToggleNorthArrow(Vector2i(1,1));
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	CHECK_EQUAL(0, pWorld->GetArrows().size());
	
	pWorld->ClearArrows();
	CHECK_EQUAL(1, pWorld->GetArrows().size());

	pWorld->ToggleNorthArrow(Vector2i(1,1));
	pWorld->AddCat(Vector2i(1,0), Direction::South);
	pWorld->Tick(2);
	CHECK_EQUAL(SquareType::HalfNorthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	pWorld->ClearArrows();
	CHECK_EQUAL(1, pWorld->GetArrows().size());

	delete pWorld;
}

TEST(ArrowsGridClear)
{
	World* pWorld = new World("Empty10x10.Level");
	pWorld->AddArrow(Direction::North);
	pWorld->ToggleNorthArrow(Vector2i(1,1));
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(1,1)));
	CHECK_EQUAL(0, pWorld->GetArrows().size());
	
	pWorld->ClearArrow(Vector2i(1,1));
	CHECK_EQUAL(1, pWorld->GetArrows().size());

	delete pWorld;
}

TEST(SolutionLoadedFromFile)
{
	World* pWorld = new World("LevelWithSolution.Level");
	CHECK_EQUAL(4, pWorld->GetArrows().size());
	pWorld->LoadSolution();
	CHECK_EQUAL(0, pWorld->GetArrows().size());
	CHECK_EQUAL(SquareType::WestArrow, pWorld->GetSquareType(Vector2i(0,0)));
	CHECK_EQUAL(SquareType::SouthArrow, pWorld->GetSquareType(Vector2i(0,3)));
	CHECK_EQUAL(SquareType::EastArrow, pWorld->GetSquareType(Vector2i(3,3)));
	CHECK_EQUAL(SquareType::NorthArrow, pWorld->GetSquareType(Vector2i(3,0)));

	
	delete pWorld;
}

TEST(CatsRespondToArrowInU)
{
	/* Checks cat responds to a south arrow in a U shape, and rotations thereof
	|C|
	|^|
	---
	*/
}

TEST(CatsAndMiceRemovableByCoordinate)
{
}

TEST(WrapAround)
{
	World* pWorld = new World("WrapAround.Level");
	CHECK_EQUAL(Vector2f(3,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::West, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,1), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,0), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());

	//Now wrap to bottom
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(1,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::East, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(2,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::East, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(3,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::East, pWorld->GetMice().at(0)->GetDirection());
	//Now wrap to left
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(0,3), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(0,2), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	pWorld->Tick(1.0f);
	CHECK_EQUAL(Vector2f(0,1), pWorld->GetMice().at(0)->GetPosition());
	CHECK_EQUAL(Direction::North, pWorld->GetMice().at(0)->GetDirection());
	//Reach a rocket
	CHECK_EQUAL(WorldState::Victory, pWorld->Tick(1.0f));
	
	delete pWorld;
}