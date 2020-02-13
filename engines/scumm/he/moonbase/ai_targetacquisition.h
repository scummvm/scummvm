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

#ifndef SCUMM_HE_MOONBASE_AI_TARGETACQUISITION_H
#define SCUMM_HE_MOONBASE_AI_TARGETACQUISITION_H

#include "scumm/he/moonbase/ai_defenseunit.h"
#include "scumm/he/moonbase/ai_node.h"
#include "scumm/he/moonbase/ai_tree.h"

namespace Scumm {

const int NUM_IMPT_UNITS = 3;
const int NUM_SHOT_POSITIONS = 1;
const int NUM_WEAPONS = 3;

class Sortie : public IContainedObject {
private:
	static int _sSourceX;
	static int _sSourceY;

	static int _sTargetX;
	static int _sTargetY;

	int _unitType;
	int _shotPosX, _shotPosY;
	Common::Array<DefenseUnit *> _enemyDefenses;
	AI *_ai;

public:
	Sortie(AI *ai) { _ai = ai; _unitType = 0; _shotPosX = _shotPosY = 0; }
	~Sortie() override;

	static void setSourcePos(int x, int y) {
		_sSourceX = x;
		_sSourceY = y;
	}
	static void setTargetPos(int x, int y) {
		_sTargetX = x;
		_sTargetY = y;
	}

	void setUnitType(int unitType) { _unitType = unitType; }

	void setShotPosX(int shotPosX) { _shotPosX = shotPosX; }
	void setShotPosY(int shotPosY) { _shotPosY = shotPosY; }
	void setShotPos(int shotPosX, int shotPosY) {
		_shotPosX = shotPosX;
		_shotPosY = shotPosY;
	}

	void setEnemyDefenses(Common::Array<DefenseUnit *> enemyDefenses) {
		_enemyDefenses = enemyDefenses;
	}
	void setEnemyDefenses(int enemyDefensesScummArray, int defendX, int defendY);

	void printEnemyDefenses();

	static int getSourcePosX() { return _sSourceX; }
	static int getSourcePosY() { return _sSourceY; }
	static int getTargetPosX() { return _sTargetX; }
	static int getTargetPosY() { return _sTargetY; }

	int getUnitType() const { return _unitType; }

	int getShotPosX() const { return _shotPosX; }
	int getShotPosY() const { return _shotPosY; }
	int *getShotPos() const;

	Common::Array<DefenseUnit *> getEnemyDefenses() const { return _enemyDefenses; }

	IContainedObject *duplicate() override;

	int numChildrenToGen() override;
	IContainedObject *createChildObj(int, int &completionFlag) override;


	float calcH() override;
	int checkSuccess() override;
	float calcT() override;
};

class Defender {
private:
	int _sourceX;
	int _sourceY;
	int _targetX;
	int _targetY;
	int _sourceUnit;
	int _power;
	int _angle;
	int _unit;
	AI *_ai;

public:
	Defender(AI *ai);
	void setSourceX(int sourceX) { _sourceX = sourceX; }
	void setSourceY(int sourceY) { _sourceY = sourceY; }
	void setTargetX(int targetX) { _targetX = targetX; }
	void setTargetY(int targetY) { _targetY = targetY; }
	void setSourceUnit(int sourceUnit) { _sourceUnit = sourceUnit; }
	void setPower(int power) { _power = power; }
	void setAngle(int angle) { _angle = angle; }
	void setUnit(int unit) { _unit = unit; }

	int getSourceX() const { return _sourceX; }
	int getSourceY() const { return _sourceY; }
	int getTargetX() const { return _targetX; }
	int getTargetY() const { return _targetY; }
	int getSourceUnit() const { return _sourceUnit; }
	int getPower() const { return _power; }
	int getAngle() const { return _angle; }
	int getUnit() const { return _unit; }

	int calculateDefenseUnitPosition(int targetX, int targetY, int index);
};

class defenseUnitCompare {
public:
	bool operator()(DefenseUnit *x, DefenseUnit *y) {
		//disabled units go at the end
		if (x->getState() == DUS_OFF) {
			debugC(DEBUG_MOONBASE_AI, "OFF");
			return 0;
		}

		return x->getDistanceTo() < y->getDistanceTo();
	}
};

} // End of namespace Scumm

#endif
