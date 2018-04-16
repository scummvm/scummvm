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

#include "common/array.h"
#include "scumm/he/moonbase/ai_tree.h"

namespace Scumm {

class ScummEngine_v100he;
class AIEntity;
class patternList;

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

class AI {
public:
	AI(ScummEngine_v100he *vm);

	void resetAI();
	void cleanUpAI();
	void setAIType(const int paramCount, const int32 *params);
	int masterControlProgram(const int paramCount, const int32 *params);

private:
	int chooseBehavior();
	int chooseTarget(int behavior);

	Tree *initApproachTarget(int targetX, int targetY, Node **retNode);
	int *approachTarget(Tree *myTree, int &x, int &y, Node **currentNode);
	Tree *initAcquireTarget(int targetX, int targetY, Node **retNode);
	int *acquireTarget(int targetX, int targetY);
	int *acquireTarget(int targetX, int targetY, Tree *myTree, int &errorCode);
	int *offendTarget(int &targetX, int &targetY, int index);
	int *defendTarget(int &targetX, int &targetY, int index);
	int *energizeTarget(int &targetX, int &targetY, int index);

public:
	int getClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled);
	int getClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled, int minDist);

	int getDistance(int originX, int originY, int endX, int endY);
	int calcAngle(int originX, int originY, int endX, int endY);
	int calcAngle(int originX, int originY, int endX, int endY, int noWrapFlag);
	int getTerrain(int x, int y);

	int getHubX(int hub);
	int getHubY(int hub);
	int getMaxX();
	int getMaxY();

	int getCurrentPlayer();
	int getMaxPower();
	int getMinPower();
	int getTerrainSquareSize();
	int getBuildingOwner(int building);
	int getBuildingState(int building);
	int getBuildingType(int building);
	int getBuildingArmor(int building);
	int getBuildingMaxArmor(int building);
	int getBuildingWorth(int building);
	int getBuildingTeam(int building);

	int getPlayerEnergy();
	int getPlayerMaxTime();
	int getTimerValue(int timerNum);
	int getPlayerTeam(int player);

	int getAnimSpeed();

	int simulateBuildingLaunch(int x, int y, int power, int angle, int numSteps, int isEnergy);

	int getPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold, int olFlag);
	int getPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold);
	int checkIfWaterState(int x, int y);
	int getUnitsWithinRadius(int x, int y, int radius);

	float degToRad(float degrees);

	int getEnergyHogType();

private:
	int getEnergyPoolsArray();
	int getCoordinateVisibility(int x, int y, int playerNum);
	int getUnitVisibility(int unit, int playerNum);
	int getEnergyPoolVisibility(int pool, int playerNum);
	int getNumberOfPools();
	int getNumberOfPlayers();
	int getWindXSpeed();
	int getWindYSpeed();
	int getTotalWindSpeed();
	int getWindXSpeedMax();
	int getWindYSpeedMax();
	int getBigXSize();
	int getBigYSize();
	int getEnergyPoolWidth(int pool);
	int getLastAttacked(int &x, int &y);
	int getFOW();
	int getBuildingStackPtr();
	int getTurnCounter();

	int getGroundAltitude(int x, int y);
	int checkForCordOverlap(int xStart, int yStart, int affectRadius, int simulateFlag);
	int checkForAngleOverlap(int unit, int angle);
	int estimateNextRoundEnergy(int player);
	int checkForUnitOverlap(int x, int y, int radius, int ignoredUnit);
	int checkForEnergySquare(int x, int y);
	int aiChat();

	int simulateWeaponLaunch(int x, int y, int power, int angle, int numSteps);
	int fakeSimulateWeaponLaunch(int x, int y, int power, int angle);

	int checkIfWaterSquare(int x, int y);

	int getLandingPoint(int x, int y, int power, int angle);
	int getEnemyUnitsVisible(int playerNum);

	void limitLocation(int &a, int &b, int c, int d);
	int energyPoolSize(int pool);
	int getMaxCollectors(int pool);

public:
	Common::Array<int> _lastXCoord[5];
	Common::Array<int> _lastYCoord[5];

	ScummEngine_v100he *_vm;

	AIEntity *_aiType[5];

	int _aiState;
	int _behavior;
	int _energyHogType;

	patternList *_moveList[5];

	const int32 *_mcpParams;
};

} // End of namespace Scumm

#endif
