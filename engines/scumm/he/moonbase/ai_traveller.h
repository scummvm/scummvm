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

#ifndef SCUMM_HE_MOONBASE_AI_TRAVELER_H
#define SCUMM_HE_MOONBASE_AI_TRAVELER_H

#include "scumm/he/moonbase/ai_node.h"

namespace Scumm {

const int NUM_TO_GEN = 9;

const int NUM_POWER_STEPS = 3;
const double SIZE_POWER_STEP = .15;
const int SIZE_ANGLE_STEP = 45;
const int VARIATION_EXTENT = 3;
const int DIRECTION_WEIGHT = 5;

class Traveller : public IContainedObject {
private:
	static int _targetPosX;
	static int _targetPosY;
	static int _maxDist;

	static int _numToGen;
	static int _sizeAngleStep;

	int _sourceHub;

	int _posX;
	int _posY;
	int _angleTo;
	int _powerTo;

	int _disabled;
	int _waterFlag;
	int _waterSourceX;
	int _waterSourceY;
	int _waterDestX;
	int _waterDestY;

	AI *_ai;

protected:
	float calcH() override;

public:
	Traveller(AI *ai);
	Traveller(int originX, int originY, AI *ai);
	~Traveller() override {}

	IContainedObject *duplicate() override { return this; }

	static void setTargetPosX(int posX) { _targetPosX = posX; }
	static void setTargetPosY(int posY) { _targetPosY = posY; }
	static void setMaxDist(int maxDist) { _maxDist = maxDist; }

	void setSourceHub(int sourceHub) { _sourceHub = sourceHub; }

	void setPosX(int posX) { _posX = posX; }
	void setPosY(int posY) { _posY = posY; }
	void setAngleTo(int angleTo) { _angleTo = angleTo; }
	void setPowerTo(int powerTo) { _powerTo = powerTo; }

	void setWaterSourceX(int waterSourceX) { _waterSourceX = waterSourceX; }
	void setWaterSourceY(int waterSourceY) { _waterSourceY = waterSourceY; }

	void setWaterDestX(int waterDestX) { _waterDestX = waterDestX; }
	void setWaterDestY(int waterDestY) { _waterDestY = waterDestY; }

	int getSourceHub() const { return _sourceHub; }

	int getPosX() const { return _posX; }
	int getPosY() const { return _posY; }
	int getAngleTo() const { return _angleTo; }
	int getPowerTo() const { return _powerTo; }

	int getWaterSourceX() const { return _waterSourceX; }
	int getWaterSourceY() const { return _waterSourceY; }
	int getWaterDestX() const { return _waterDestX; }
	int getWaterDestY() const { return _waterDestY; }

	void setDisabled() { _disabled = 1; }
	void unsetDisabled() { _disabled = 0; }
	int getDisabled() { return _disabled; }

	void adjustPosX(int offsetX);
	void adjustPosY(int offsetY);
	void adjustXY(int offsetX, int offsetY);

	void enableWaterFlag() { _waterFlag = 1; }
	void disableWaterFlag() { _waterFlag = 0; }
	int getWaterFlag() const { return _waterFlag; }

	int numChildrenToGen() override;
	IContainedObject *createChildObj(int, int &) override;

	int checkSuccess() override;
	float calcT() override;
};

} // End of namespace Scumm

#endif
