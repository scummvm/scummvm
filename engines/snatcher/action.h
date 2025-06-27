/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef SNATCHER_ACTION_H
#define SNATCHER_ACTION_H

#include "common/scummsys.h"

namespace Common {

} // namespace Common

namespace Snatcher {

class ResourcePointer;
class SnatcherEngine;
class ActionSequenceHandler {
public:
	ActionSequenceHandler(SnatcherEngine *vm, ResourcePointer *scd);
	~ActionSequenceHandler();

	bool run(bool useLightGun);
	bool resetScore();
	void setup(uint16 stage);
	void setDifficulty(uint16 setting);

	struct Result {
		Result() : hitsTaken(0), shotsFired(0), shotsMissed(0), enemiesShot(0), civiliansShot(0), allCiviliansShot(0), weaponDrawn(0)  {}
		uint8 hitsTaken;
		uint8 shotsFired;
		uint8 shotsMissed;
		uint8 enemiesShot;
		uint8 civiliansShot;
		uint8 allCiviliansShot;
		uint8 weaponDrawn;
	};

	const Result &getResult() const { return _result; }

private:
	void resetVars();
	void initStage();
	void handleWeaponDrawAndTargetSelection();
	void handleShot();
	void updateEnemies();
	void updateStage();
	void enemyAttack();
	void checkFinished();

	uint16 _stage;
	int16 _progressMSB;
	int16 _progressLSB;
	int16 _weaponState;
	int16 _enemySpeed;
	int16 _handleShotPhase;
	int16 _updateEnemiesPhase;
	int16 _enemyAttackStatus;
	int16 _targetField;
	int16 _hscrollB;
	int16 _vscrollB;
	int16 _enemyAppearanceCount;
	int16 _shoot_7912;
	int16 _civiliansShot;
	int16 _civilianAppearanceCount;
	int16 _abortFight;
	int16 _nextEnemyAppearanceTimer;
	int16 _animNo;
	int16 _enemyAttackTimer;
	int16 _shoot_78EE;
	int16 _counter1;
	int16 _counter2;
	int16 _shoot_78EC;
	int16 _attackerId;
	int16 *_projectileCoords;
	bool _active;

	SnatcherEngine *_vm;
	ResourcePointer *_scd;
	bool _useLightGun;

	Result _result;
};

} // End of namespace Snatcher

#endif // SNATCHER_ACTION_H
