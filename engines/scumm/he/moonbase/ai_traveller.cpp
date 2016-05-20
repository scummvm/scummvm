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

#include "scumm/he/moonbase/ai_traveller.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

int Traveller::_targetPosX = 0;
int Traveller::_targetPosY = 0;
int Traveller::_maxDist = 0;

int Traveller::_numToGen = 0;
int Traveller::_sizeAngleStep = 0;

Traveller::Traveller() {
	_waterFlag = 0;
	setValueG(0);
	unsetDisabled();
}

Traveller::Traveller(int originX, int originY) {
	_waterFlag = 0;
	setValueG(0);
	unsetDisabled();

	_posX = originX;
	_posY = originY;
}

void Traveller::adjustPosX(int offsetX) {
	int maxX = GetMaxX();
	int deltaX = _posX + offsetX;

	if (deltaX < 0) _posX = maxX + deltaX;
	else if (deltaX > maxX) _posX = deltaX - maxX;
	else _posX = deltaX;
}

void Traveller::adjustPosY(int offsetY) {
	int maxY = GetMaxX();
	int deltaY = _posY + offsetY;

	if (deltaY < 0) _posY = maxY + deltaY;
	else if (deltaY > maxY) _posY = deltaY - maxY;
	else _posY = deltaY;
}

void Traveller::adjustXY(int offsetX, int offsetY) {
	adjustPosX(offsetX);
	adjustPosY(offsetY);
}

float Traveller::calcH() {
	float retVal = 0;
	// Calc dist from here to target
	retVal = GetDistance(_posX, _posY, _targetPosX, _targetPosY);
	// Divide by _maxDist to get minimum number of jumps to goal
	retVal /= static_cast<float>(_maxDist);

	return retVal * 2.0;
}

int Traveller::numChildrenToGen() {
	if (!_numToGen)
		_numToGen = GetAnimSpeed() + 2;

	return _numToGen;
}

IContainedObject *Traveller::createChildObj(int index, int &completionFlag) {
	static int nodeCount = 0;
	static int completionState = 1;

	if (!index) nodeCount = 0;

	nodeCount++;

	Traveller *retTraveller = new Traveller;

	static int dir, angle, power;

	if (completionState) {
		// Calculate angle between here and target
		int directAngle = 0;

		if (GetEnergyHogType())
			directAngle = GetAngle(_posX, _posY, _targetPosX, _targetPosY, 1);
		else
			directAngle = GetAngle(_posX, _posY, _targetPosX, _targetPosY);

		// Calculate the offset angle for this index
		if (!_sizeAngleStep)
			_sizeAngleStep = 52 - (GetAnimSpeed() * 7);

		dir = _sizeAngleStep * ((static_cast<int>(index / NUM_POWER_STEPS) + 1) >> 1);
		// Calculate the sign value for the offset for this index
		int orientation = dir * (((static_cast<int>(index / NUM_POWER_STEPS) % 2) << 1) - 1);
		// Add the offset angle to the direct angle to target
		angle = orientation + directAngle;

		// Calculate power for this index
		int maxPower = 0;
		int directDist = GetDistance(_posX, _posY, _targetPosX, _targetPosY);

		if (directDist > _maxDist + 120)
			maxPower = GetMaxPower();
		else
			maxPower = (static_cast<float>(directDist) / static_cast<float>(_maxDist + 120)) * GetMaxPower();

		maxPower -= 70;
		power = maxPower * (1 - ((index % NUM_POWER_STEPS) * SIZE_POWER_STEP));
	}

	retTraveller->setAngleTo(angle);
	retTraveller->setPowerTo(power);

	// Set this object's position to the new one determined by the power and angle from above
	static int lastSuccessful = 0;
	int coords = 0;

	if (!(index % NUM_POWER_STEPS) || (!lastSuccessful)) {
		coords = SimulateBuildingLaunch(_posX, _posY, power, angle, 10, 0);
		lastSuccessful = 0;
	} else {
		completionState = 1;
		lastSuccessful = 0;
	}

	if (!coords) {
		completionFlag = 0;
		completionState = 0;
		delete retTraveller;
		return NULL;
	} else {
		completionFlag = 1;
		completionState = 1;
	}

	int whoseTurn = GetCurrentPlayer();
	int maxX = GetMaxX();

	// Check new position to see if landing is clear
	if (coords > 0) {
		int yCoord = coords / maxX;
		int xCoord = coords - (yCoord * maxX);

		int terrain = GetTerrain(xCoord, yCoord);
		assert(terrain == TERRAIN_TYPE_GOOD);

		float pwr = GetMinPower() * .3;
		float cosine = cos((static_cast<float>(angle) / 360) * (2 * M_PI));
		float sine = sin((static_cast<float>(angle) / 360) * (2 * M_PI));
		int xParam = xCoord + (pwr * cosine);
		int yParam = yCoord + (pwr * sine);

		if (xParam < 0)
			xParam += GetMaxX();
		else if (xParam > GetMaxX())
			xParam -= GetMaxX();

		if (yParam < 0)
			yParam += GetMaxY();
		else if (yParam > GetMaxY())
			yParam -= GetMaxY();

		if (CheckIfWaterState(xParam, yParam)) {
			delete retTraveller;
			return NULL;
		}

		retTraveller->setPosY(yCoord);
		retTraveller->setPosX(xCoord);

		// Iterate through the previous action list, making sure this one isn't on it
		for (intVecItr i = (lastXCoord[whoseTurn]).begin(), j = (lastYCoord[whoseTurn]).begin(); i != (lastXCoord[whoseTurn]).end(); ++i, ++j) {
			// Check if this shot is the same as the last time we tried
			if ((*i == retTraveller->getPosX()) && (*j == retTraveller->getPosY())) {
				retTraveller->setDisabled();
				delete retTraveller;
				return NULL;
			}
		}

		retTraveller->setValueG(getG() + 7 + (dir * DIRECTION_WEIGHT));
		lastSuccessful = 1;
	} else {
		int yCoord  = -coords / maxX;
		int xCoord = -coords - (yCoord * maxX);

		// If landing fault is because of water, add 1 extra to g and turn on water flag.  Also set coords, and adjust power to water fault location
		if (CheckIfWaterState(xCoord, yCoord)) {
			int terrainSquareSize = GetTerrainSquareSize();
			xCoord = ((xCoord / terrainSquareSize * terrainSquareSize) + (terrainSquareSize / 2));
			yCoord = ((yCoord / terrainSquareSize * terrainSquareSize) + (terrainSquareSize / 2));

			int xDist = xCoord - _posX;
			int yDist = yCoord - _posY;
			retTraveller->setPosX(xCoord + (terrainSquareSize * 1.414 * (xDist / (abs(xDist) + 1))));
			retTraveller->setPosY(yCoord + (terrainSquareSize * 1.414 * (yDist / (abs(yDist) + 1))));

			int closestHub = GetClosestUnit(retTraveller->getPosX(), retTraveller->getPosY(), GetMaxX(), GetCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 110);

			retTraveller->setWaterSourceX(GetHubX(closestHub));
			retTraveller->setWaterSourceY(GetHubY(closestHub));
			retTraveller->setWaterDestX(retTraveller->getPosX());
			retTraveller->setWaterDestY(retTraveller->getPosY());

			retTraveller->setPowerTo(power);
			retTraveller->setAngleTo(angle);

			retTraveller->setValueG(getG() + 10 + (dir * DIRECTION_WEIGHT));
			retTraveller->enableWaterFlag();
		} else {
			// If not, set G to highest value
			retTraveller->setDisabled();
			delete retTraveller;
			return NULL;
		}
	}

	return retTraveller;
}

int Traveller::checkSuccess() {
	if (GetDistance(_posX + 1, _posY, _targetPosX, _targetPosY) < _maxDist)
		return SUCCESS;

	return 0;
}

float Traveller::calcT() {
	assert(!_disabled);

	if (_disabled) return FAILURE;

	return (checkSuccess() != SUCCESS) ? (getG() + calcH()) : SUCCESS;
}

} // End of namespace Scumm
