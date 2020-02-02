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

#ifndef WORLD_ACTORS_COMBATPROCESS_H
#define WORLD_ACTORS_COMBATPROCESS_H

#include "ultima/ultima8/kernel/process.h"

namespace Ultima {
namespace Ultima8 {

class Actor;

class CombatProcess : public Process {
public:
	CombatProcess();
	CombatProcess(Actor *actor);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	virtual void run() override;

	virtual void terminate() override;

	ObjId getTarget();
	void setTarget(ObjId target_);
	ObjId seekTarget();

	virtual void dumpInfo() override;

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods) override;

	bool isValidTarget(Actor *target_);
	bool isEnemy(Actor *target_);
	bool inAttackRange();
	int getTargetDirection();

	void turnToDirection(int direction);
	void waitForTarget();

	ObjId target;
	ObjId fixedTarget;

	enum CombatMode {
		CM_WAITING = 0,
		CM_PATHFINDING,
		CM_ATTACKING
	} combatmode;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
