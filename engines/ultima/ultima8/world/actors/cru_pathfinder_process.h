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

#ifndef WORLD_ACTORS_CRU_PATHFINDERPROCESS_H
#define WORLD_ACTORS_CRU_PATHFINDERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/point3.h"

namespace Ultima {
namespace Ultima8 {

class Actor;
class Item;

/**
 * A simplified pathfinder used in Crusader for the AttackProcess.
 *
 * The code and variable names for this are not very well written as
 * they are are based on the disassembly.
 */
class CruPathfinderProcess : public Process {
public:
	CruPathfinderProcess();
	CruPathfinderProcess(Actor *actor, Item *item, int maxsteps, int stopdistance, bool turnatend);
	CruPathfinderProcess(Actor *actor, const Point3 &target, int maxsteps, int stopdistance, bool turnatend);
	~CruPathfinderProcess() override;

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;
	void terminate() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:

	Direction nextDirFromPoint(struct Point3 &npcpt);

	Point3 _target;
	ObjId _targetItem;
	int _currentDistance;
	bool _randomFlag;
	bool _nextTurn;
	bool _turnAtEnd;

	Direction _lastDir;
	Direction _nextDir;
	Direction _nextDir2;

	bool _solidObject;
	bool _directPathBlocked;
	bool _noShotAvailable;
	bool _dir16Flag;

	unsigned int _currentStep;
	unsigned int _maxSteps;
	int _stopDistance;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
