#include "stdafx.h"
#include <GridSquare.h>

TEST(CheckGridSquare)
{
	GridSquare gs;
	CHECK_EQUAL(gs.GetNorth(), false);
	CHECK_EQUAL(gs.GetSouth(), false);
	CHECK_EQUAL(gs.GetWest(), false);
	CHECK_EQUAL(gs.GetEast(), false);

	gs.SetNorth(true);
	gs.SetSouth(true);
	gs.SetEast(true);
	gs.SetWest(true);
	CHECK_EQUAL(gs.GetNorth(), true);
	CHECK_EQUAL(gs.GetSouth(), true);
	CHECK_EQUAL(gs.GetWest(), true);
	CHECK_EQUAL(gs.GetEast(), true);

	gs.Clear();
	CHECK_EQUAL(gs.GetNorth(), false);
	CHECK_EQUAL(gs.GetSouth(), false);
	CHECK_EQUAL(gs.GetWest(), false);
	CHECK_EQUAL(gs.GetEast(), false);

	//Check the turned directions are correct
	gs.SetNorth();
	CHECK_EQUAL(Direction::East, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::South, gs.GetTurnDirection(Direction::South));
	CHECK_EQUAL(Direction::West, gs.GetTurnDirection(Direction::West));
	CHECK_EQUAL(Direction::East, gs.GetTurnDirection(Direction::East));
	gs.SetEast();
	CHECK_EQUAL(Direction::West, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::South, gs.GetTurnDirection(Direction::East));
	CHECK_EQUAL(Direction::South, gs.GetTurnDirection(Direction::South));
	gs.SetWest();
	CHECK_EQUAL(Direction::South, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::South, gs.GetTurnDirection(Direction::South));

	gs.Clear();
	gs.SetSouth();
	CHECK_EQUAL(Direction::West, gs.GetTurnDirection(Direction::South));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::West, gs.GetTurnDirection(Direction::West));
	CHECK_EQUAL(Direction::East, gs.GetTurnDirection(Direction::East));
	gs.SetWest();
	CHECK_EQUAL(Direction::East, gs.GetTurnDirection(Direction::South));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::West));
	CHECK_EQUAL(Direction::East, gs.GetTurnDirection(Direction::East));
	gs.SetEast();
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::South));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::West));
	CHECK_EQUAL(Direction::North, gs.GetTurnDirection(Direction::East));
}

TEST(DirectionFromString)
{
	CHECK_EQUAL(Direction::North, Direction::FromString("North"));
	CHECK_EQUAL(Direction::West, Direction::FromString("West"));
	CHECK_EQUAL(Direction::East, Direction::FromString("East"));
	CHECK_EQUAL(Direction::South, Direction::FromString("South"));
}

TEST(StopWhenTrapped)
{
	GridSquare gs = GridSquare(true, true, true, true);
	CHECK_EQUAL(Direction::Stopped, gs.GetTurnDirection(Direction::North));
	CHECK_EQUAL(Direction::Stopped, gs.GetTurnDirection(Direction::South));
	CHECK_EQUAL(Direction::Stopped, gs.GetTurnDirection(Direction::East));
	CHECK_EQUAL(Direction::Stopped, gs.GetTurnDirection(Direction::West));
}

TEST(ToggleWalls)
{
	GridSquare gs(false, false, false, false);
	gs.ToggleDirection(Direction::North);
	CHECK_EQUAL(true, gs.GetDirection(Direction::North));
}