/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCUMM_HE_MOONBASE_AI_MAIN_H
#define SCUMM_HE_MOONBASE_AI_MAIN_H

#include "scumm/he/moonbase/ai_tree.h"

namespace Scumm {

class ScummEngine_v71he;

extern ScummEngine_v71he *_vm;

typedef Common::Array<int>::iterator intVecItr;

enum {
	TERRAIN_TYPE_GOOD = 0,
	TERRAIN_TYPE_SLOPE = 1,
	TERRAIN_TYPE_WATER = 2,
	MAX_MEMORY = 3
};

enum {
	ITEM_BOMB = 0,
	ITEM_CLUSTER = 1,
	ITEM_REPAIR = 2,
	ITEM_ANTIAIR = 3,
	ITEM_BRIDGE = 4,
	ITEM_TOWER = 5,
	ITEM_GUIDED = 6,
	ITEM_EMP = 7,
	ITEM_SPIKE = 8,
	ITEM_RECLAIMER = 9,
	ITEM_BALLOON = 10,
	ITEM_MINE = 11,
	ITEM_CRAWLER = 12,
	ITEM_VIRUS = 13,
	ITEM_ENERGY = 14,
	ITEM_SHIELD = 15,
	ITEM_OFFENSE = 16,
	ITEM_HUB = 17,
	ITEM_TIME_EXPIRED = 18,
	SKIP_TURN = -999
};

enum BuildingTypes {
	BUILDING_ENERGY_COLLECTOR = 3,
	BUILDING_MAIN_BASE = 4,
	BUILDING_BRIDGE = 5,
	BUILDING_TOWER = 6,
	BUILDING_EXPLOSIVE_MINE = 7,
	BUILDING_SHIELD = 8,
	BUILDING_ANTI_AIR = 9,
	BUILDING_OFFENSIVE_LAUNCHER = 10,
	BUILDING_BALLOON   = 11,
	BUILDING_CRAWLER   = 12
};

enum {
	ENERGY_POOL_X = 45,
	ENERGY_POOL_Y = 46,
	ENERGY_POOL_UNITS_ON = 47,

	MIN_DIST = 108
};

static int energyHogType = 0;

void ResetAI();
void CleanUpAI();
void SetAIType(const int paramCount, const int *params);
int MasterControlProgram(const int paramCount, const int *params);

int ChooseBehavior();
int ChooseTarget(int behavior);

Tree *InitApproachTarget(int targetX, int targetY, Node **retNode);
int *ApproachTarget(Tree *myTree, int &x, int &y, Node **currentNode);
Tree *InitAcquireTarget(int targetX, int targetY, Node **retNode);
int *AcquireTarget(int targetX, int targetY);
int *AcquireTarget(int targetX, int targetY, Tree *myTree, int &errorCode);
int *OffendTarget(int &targetX, int &targetY, int index);
int *DefendTarget(int &targetX, int &targetY, int index);
int *EnergizeTarget(int &targetX, int &targetY, int index);

int GetClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled);
int GetClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled, int minDist);

int GetDistance(int originX, int originY, int endX, int endY);
int GetAngle(int originX, int originY, int endX, int endY);
int GetAngle(int originX, int originY, int endX, int endY, int noWrapFlag);
int GetTerrain(int x, int y);
int GetHubX(int hub);
int GetHubY(int hub);
int GetMaxX();
int GetMaxY();
int GetCurrentPlayer();
int GetMaxPower();
int GetMinPower();
int GetTerrainSquareSize();
int GetBuildingOwner(int building);
int GetBuildingState(int building);
int GetBuildingType(int building);
int GetBuildingArmor(int building);
int GetBuildingWorth(int building);
void DebugBreak();
int GetEnergyPoolsArray();
int GetCoordinateVisibility(int x, int y, int playerNum);
int GetUnitVisibility(int unit, int playerNum);
int GetEnergyPoolVisibility(int pool, int playerNum);
int GetNumberOfPools();
int GetNumberOfPlayers();
int GetPlayerEnergy();
int GetPlayerMaxTime();
int GetWindXSpeed();
int GetWindYSpeed();
int GetTotalWindSpeed();
int GetWindXSpeedMax();
int GetWindYSpeedMax();
int GetBigXSize();
int GetBigYSize();
int GetEnergyPoolWidth(int pool);
int GetBuildingMaxArmor(int building);
int GetTimerValue(int timerNum);
int GetLastAttacked(int &x, int &y);
int PrintDebugTimer(int max, int timerVal);
int GetPlayerTeam(int player);
int GetBuildingTeam(int building);
int GetFOW();
int GetAnimSpeed();
int GetBuildingStackPtr();
int GetTurnCounter();

int GetGroundAltitude(int x, int y);
int CheckForCordOverlap(int xStart, int yStart, int affectRadius, int simulateFlag);
int CheckForAngleOverlap(int unit, int angle);
int EstimateNextRoundEnergy(int player);
int CheckForUnitOverlap(int x, int y, int radius, int ignoredUnit);
int CheckForEnergySquare(int x, int y);
int AIChat();


int SimulateBuildingLaunch(int x, int y, int power, int angle, int numSteps, int isEnergy);
int SimulateWeaponLaunch(int x, int y, int power, int angle, int numSteps);
int FakeSimulateWeaponLaunch(int x, int y, int power, int angle);

int GetPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold, int olFlag);
int GetPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold);
int CheckIfWaterState(int x, int y);
int CheckIfWaterSquare(int x, int y);
int GetUnitsWithinRadius(int x, int y, int radius);
int GetLandingPoint(int x, int y, int power, int angle);
int GetEnemyUnitsVisible(int playerNum);

float degToRad(float degrees);
void MACRO_LimitLocation(int &a, int &b, int c, int d);
int energyPoolSize(int pool);
int GetMaxCollectors(int pool);

int TempChooseBehavior();
int TempChooseTarget(int behavior);

int GetEnergyHogType();

extern Common::Array<int> lastXCoord[];
extern Common::Array<int> lastYCoord[];

} // End of namespace Scumm

#endif
