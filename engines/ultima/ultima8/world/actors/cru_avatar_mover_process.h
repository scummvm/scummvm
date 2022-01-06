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

#ifndef WORLD_ACTORS_CRUAVATARMOVERPROCESS_H
#define WORLD_ACTORS_CRUAVATARMOVERPROCESS_H

#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/animation.h"

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

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	double getAvatarAngleDegrees() const {
		return static_cast<double>(_avatarAngle) / 100.0;
	}

	void clearMovementFlag(uint32 mask) override;

private:
	/** Try readying or firing weapon. */
	void tryAttack();

	/** Check if we need to alert NPCs after firing. */
	void checkForAlertingNPCs();

	/**
	* Angle of avatar in centidegrees (1/100deg).  The original game runs the keyboard
	* process 45 times per second and rotates the crosshair by 2 (regular) or
	* 5 ('run') degrees each time. This process runs 60 times per second, so we choose a
	* multiplier that can use integers - rotating 3.75 or 1.5 degrees each time.
	*/
	int32 _avatarAngle;

	/**
	 * Whether we've reloaded the SGA1 yet (it needs to happen every shot)
	 */
	bool _SGA1Loaded;

	/**
	 * Next tick the avatar can fire a weapon again.
	 */
	uint32 _nextFireTick;

	/**
	 * Last time we alerted NPCs on a shot.
	 */
	uint32 _lastNPCAlertTick;

	void handleHangingMode() override;
	void handleCombatMode() override;
	void handleNormalMode() override;

	void step(Animation::Sequence action, Direction direction, bool adjusted = false);

	bool checkOneShotMove(Direction direction);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
