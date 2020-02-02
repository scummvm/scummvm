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

#ifndef WORLD_ACTORS_AVATARMOVERPROCESS_H
#define WORLD_ACTORS_AVATARMOVERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima {
namespace Ultima8 {

class AvatarMoverProcess : public Process {
public:
	AvatarMoverProcess();
	virtual ~AvatarMoverProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	virtual void run();

	void OnMouseDown(int button, int32 mx, int32 my);
	void OnMouseUp(int button);

	void resetIdleTime() {
		idleTime = 0;
	}

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

	void handleHangingMode();
	void handleCombatMode();
	void handleNormalMode();

	void step(Animation::Sequence action, int direction, bool adjusted = false);
	void jump(Animation::Sequence action, int direction);
	void turnToDirection(int direction);
	bool checkTurn(int direction, bool moving);
	bool canAttack();

	uint32 lastframe;

	// attack speed limiting
	uint32 lastAttack;

	// shake head when idle
	uint32 idleTime;
	Animation::Sequence lastHeadShakeAnim;

	MButton mouseButton[2];
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
