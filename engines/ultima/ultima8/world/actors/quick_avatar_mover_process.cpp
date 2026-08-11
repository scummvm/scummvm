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

#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(QuickAvatarMoverProcess)

ProcId QuickAvatarMoverProcess::_amp = 0;
bool QuickAvatarMoverProcess::_enabled = false;
bool QuickAvatarMoverProcess::_clipping = false;

QuickAvatarMoverProcess::QuickAvatarMoverProcess() : Process(1), _movementFlags(0) {
	_amp = getPid();
}

QuickAvatarMoverProcess::~QuickAvatarMoverProcess() {
}

void QuickAvatarMoverProcess::run() {
	if (!isEnabled()) {
		terminate();
		return;
	}

	MainActor *avatar = getMainActor();
	Direction direction = avatar->getDir();
	DirectionMode mode = GAME_IS_U8 ? dirmode_8dirs : dirmode_16dirs;

	int32 dx = 0;
	int32 dy = 0;
	int32 dz = 0;

	if (hasMovementFlags(MOVE_UP)) {
		dx -= 64;
		dy -= 64;
	}

	if (hasMovementFlags(MOVE_DOWN)) {
		dx += 64;
		dy += 64;
	}

	if (hasMovementFlags(MOVE_LEFT)) {
		dx -= 64;
		dy += 64;
	}

	if (hasMovementFlags(MOVE_RIGHT)) {
		dx += 64;
		dy -= 64;
	}

	if (hasMovementFlags(MOVE_ASCEND)) {
		dz += 8;
	}

	if (hasMovementFlags(MOVE_DESCEND)) {
		dz -= 8;
	}

	// Limit speed of turning by checking
	uint32 frameNum = Kernel::get_instance()->getFrameNum();
	if (frameNum % 4 == 0) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			direction = Direction_OneLeft(direction, mode);
		}

		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			direction = Direction_OneRight(direction, mode);
		}
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		int xoff = 32 * Direction_XFactor(direction);
		int yoff = 32 * Direction_YFactor(direction);
		if (dirmode_8dirs) {
			xoff *= 2;
			yoff *= 2;
		}

		dx += xoff;
		dy += yoff;
	}

	if (hasMovementFlags(MOVE_BACK)) {
		int xoff = 32 * Direction_XFactor(direction);
		int yoff = 32 * Direction_YFactor(direction);
		if (dirmode_8dirs) {
			xoff *= 2;
			yoff *= 2;
		}

		dx -= xoff;
		dy -= yoff;
	}

	if (direction != avatar->getDir()) {
		avatar->setDir(direction);
		avatar->setToStartOfAnim(Animation::stand);
	}

	if (!dx && !dy && !dz) {
		return;
	}

	if (hasMovementFlags(MOVE_SLOW)) {
		dx /= 4;
		dy /= 4;
		dz /= 4;
	} else if (hasMovementFlags(MOVE_FAST)) {
		dx *= 4;
		dy *= 4;
		dz *= 4;
	}

	Point3 pt = avatar->getLocation();
	int32 ixd, iyd, izd;
	avatar->getFootpadWorld(ixd, iyd, izd);

	CurrentMap *cm = World::get_instance()->getCurrentMap();
	int32 dxv = dx;
	int32 dyv = dy;
	int32 dzv = dz;

	if (_clipping) {
		for (int j = 0; j < 3; j++) {
			dxv = dx;
			dyv = dy;
			dzv = dz;

			if (j == 1)
				dxv = 0;
			else if (j == 2)
				dyv = 0;

			bool ok = false;

			while (dxv || dyv || dzv) {
				uint32 shapeFlags = avatar->getShapeInfo()->_flags;

				Box start(pt.x, pt.y, pt.z, ixd, iyd, izd);
				PositionInfo info = cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z + dzv, ixd, iyd, izd), start, shapeFlags, 1);
				if (info.valid) {
					if (!dzv && !info.supported) {
						// Adjust to stay on ground
						if (cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z - 8, ixd, iyd, izd), start, shapeFlags, 1).valid &&
							!cm->getPositionInfo(Box(pt.x, pt.y, pt.z - 8, ixd, iyd, izd), start, shapeFlags, 1).valid) {
							dzv = -8;
						} else if (cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z - 16, ixd, iyd, izd), start, shapeFlags, 1).valid &&
								   !cm->getPositionInfo(Box(pt.x, pt.y, pt.z - 16, ixd, iyd, izd), start, shapeFlags, 1).valid) {
							dzv = -16;
						} else if (cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z - 24, ixd, iyd, izd), start, shapeFlags, 1).valid &&
								   !cm->getPositionInfo(Box(pt.x, pt.y, pt.z - 24, ixd, iyd, izd), start, shapeFlags, 1).valid) {
							dzv = -24;
						} else if (cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z - 32, ixd, iyd, izd), start, shapeFlags, 1).valid &&
								   !cm->getPositionInfo(Box(pt.x, pt.y, pt.z - 32, ixd, iyd, izd), start, shapeFlags, 1).valid) {
							dzv = -32;
						}
					}
					ok = true;
					break;
				} else if (cm->getPositionInfo(Box(pt.x + dxv, pt.y + dyv, pt.z + dzv + 8, ixd, iyd, izd), start, shapeFlags, 1).valid) {
					dzv += 8;
					ok = true;
					break;
				}
				dxv /= 2;
				dyv /= 2;
				dzv /= 2;
			}

			if (ok)
				break;
		}
	}

	// Yes, i know, not entirely correct
	avatar->collideMove(pt.x + dxv, pt.y + dyv, pt.z + dzv, false, true);

	if (GAME_IS_CRUSADER) {
		// Keep the camera on the avatar while we're quick-moving.
		Point3 cpt(pt.x + dxv, pt.y + dyv, pt.z + dzv);
		CameraProcess::SetCameraProcess(new CameraProcess(cpt));
	}

	// Prevent avatar from running an idle animation while moving around
	Ultima8Engine::get_instance()->getAvatarMoverProcess()->resetIdleTime();
}

void QuickAvatarMoverProcess::terminate() {
	Process::terminate();
	_amp = 0;
}

QuickAvatarMoverProcess *QuickAvatarMoverProcess::get_instance() {
	Kernel *kernel = Kernel::get_instance();
	QuickAvatarMoverProcess *p = nullptr;
	if (_amp) {
		p = dynamic_cast<QuickAvatarMoverProcess *>(kernel->getProcess(_amp));
	}

	if (!p) {
		p = new QuickAvatarMoverProcess();
		Kernel::get_instance()->addProcess(p);
	}

	return p;
}

bool QuickAvatarMoverProcess::onActionDown(KeybindingAction action) {
	if (!isEnabled()) {
		return false;
	}

	bool handled = true;
	switch (action) {
	case ACTION_MOVE_ASCEND:
		setMovementFlag(MOVE_ASCEND);
		break;
	case ACTION_MOVE_DESCEND:
		setMovementFlag(MOVE_DESCEND);
		break;
	case ACTION_TURN_LEFT:
		setMovementFlag(MOVE_TURN_LEFT);
		break;
	case ACTION_TURN_RIGHT:
		setMovementFlag(MOVE_TURN_RIGHT);
		break;
	case ACTION_MOVE_FORWARD:
		setMovementFlag(MOVE_FORWARD);
		break;
	case ACTION_MOVE_BACK:
		setMovementFlag(MOVE_BACK);
		break;
	case ACTION_MOVE_UP:
		setMovementFlag(MOVE_UP);
		break;
	case ACTION_MOVE_DOWN:
		setMovementFlag(MOVE_DOWN);
		break;
	case ACTION_MOVE_LEFT:
		setMovementFlag(MOVE_LEFT);
		break;
	case ACTION_MOVE_RIGHT:
		setMovementFlag(MOVE_RIGHT);
		break;
	case ACTION_MOVE_STEP:
		setMovementFlag(MOVE_SLOW);
		// Allow others to handle as well
		handled = false; 
		break;
	case ACTION_MOVE_RUN:
		setMovementFlag(MOVE_FAST);
		// Allow others to handle as well
		handled = false; 
		break;
	default:
		handled = false;
	}
	return handled;
}

bool QuickAvatarMoverProcess::onActionUp(KeybindingAction action) {
	if (!isEnabled()) {
		return false;
	}

	bool handled = true;
	switch (action) {
	case ACTION_MOVE_ASCEND:
		clearMovementFlag(MOVE_ASCEND);
		break;
	case ACTION_MOVE_DESCEND:
		clearMovementFlag(MOVE_DESCEND);
		break;
	case ACTION_TURN_LEFT:
		clearMovementFlag(MOVE_TURN_LEFT);
		break;
	case ACTION_TURN_RIGHT:
		clearMovementFlag(MOVE_TURN_RIGHT);
		break;
	case ACTION_MOVE_FORWARD:
		clearMovementFlag(MOVE_FORWARD);
		break;
	case ACTION_MOVE_BACK:
		clearMovementFlag(MOVE_BACK);
		break;
	case ACTION_MOVE_UP:
		clearMovementFlag(MOVE_UP);
		break;
	case ACTION_MOVE_DOWN:
		clearMovementFlag(MOVE_DOWN);
		break;
	case ACTION_MOVE_LEFT:
		clearMovementFlag(MOVE_LEFT);
		break;
	case ACTION_MOVE_RIGHT:
		clearMovementFlag(MOVE_RIGHT);
		break;
	case ACTION_MOVE_STEP:
		clearMovementFlag(MOVE_SLOW);
		// Allow others to handle as well
		handled = false; 
		break;
	case ACTION_MOVE_RUN:
		clearMovementFlag(MOVE_FAST);
		// Allow others to handle as well
		handled = false; 
		break;
	default:
		handled = false;
	}
	return handled;
}


void QuickAvatarMoverProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_movementFlags);
	// don't save more information. We plan to terminate upon load
}

bool QuickAvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_movementFlags = rs->readUint32LE();
	terminateDeferred(); // Don't allow this process to continue
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
