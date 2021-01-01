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

#ifndef WORLD_ACTORS_CRUAVATARMOVERPROCESS_H
#define WORLD_ACTORS_CRUAVATARMOVERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Mover process that replicates the feel of Crusader - moving, combat, jumps, etc.
 * Tries sliding left and right if movement is blocked.  Walking cancels combat.
 * TODO: Support combat rolls and side-steps.
 */
class CruAvatarMoverProcess : public AvatarMoverProcess {
public:
	CruAvatarMoverProcess();
	~CruAvatarMoverProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	void tryAttack() override;

	double getAvatarAngleDegrees() const {
		return static_cast<double>(_avatarAngle) / 100.0;
	}

private:

	/**
	* Angle of avatar in centidegrees (1/100deg).  The original game runs the keyboard
	* process 45 times per second and rotates the crosshair by 2 (regular) or
	* 5 ('run') degrees each time. This process runs 60 times per second, so we choose a
	* multiplier that can use integers - rotating 3.75 or 1.5 degrees each time.
	*/
	int32 _avatarAngle;

	void handleHangingMode() override;
	void handleCombatMode() override;
	void handleNormalMode() override;
	bool canAttack() override;

	void step(Animation::Sequence action, Direction direction, bool adjusted = false);

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
