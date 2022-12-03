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
bool QuickAvatarMoverProcess::_clipping = false;
bool QuickAvatarMoverProcess::_quarter = false;

QuickAvatarMoverProcess::QuickAvatarMoverProcess() : Process(1), _movementFlags(0) {
	_amp = getPid();
}

QuickAvatarMoverProcess::~QuickAvatarMoverProcess() {
}

void QuickAvatarMoverProcess::run() {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		terminate();
		return;
	}

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

	if (!dx && !dy && !dz) {
		return;
	}

	MainActor *avatar = getMainActor();
	int32 x, y, z;
	avatar->getLocation(x, y, z);
	int32 ixd, iyd, izd;
	avatar->getFootpadWorld(ixd, iyd, izd);

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	int32 dxv = dx;
	int32 dyv = dy;
	int32 dzv = dz;

	for (int j = 0; j < 3; j++) {
		dxv = dx;
		dyv = dy;
		dzv = dz;

		if (j == 1) dxv = 0;
		else if (j == 2) dyv = 0;

		if (_quarter) {
			dxv /= 4;
			dyv /= 4;
			dzv /= 4;
		}

		bool ok = false;

		while (dxv || dyv || dzv) {
			uint32 shapeFlags = avatar->getShapeInfo()->_flags;

			if (!_clipping || cm->isValidPosition(x + dxv, y + dyv, z + dzv, ixd, iyd, izd, _flags, 1, 0, 0)) {
				if (_clipping && !dzv) {
					if (cm->isValidPosition(x + dxv, y + dyv, z - 8, ixd, iyd, izd, _flags, 1, 0, 0) &&
					        !cm->isValidPosition(x, y, z - 8, ixd, iyd, izd, _flags, 1, 0, 0)) {
						dzv = -8;
					} else if (cm->isValidPosition(x + dxv, y + dyv, z - 16, ixd, iyd, izd, _flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 16, ixd, iyd, izd, _flags, 1, 0, 0)) {
						dzv = -16;
					} else if (cm->isValidPosition(x + dxv, y + dyv, z - 24, ixd, iyd, izd, _flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 24, ixd, iyd, izd, _flags, 1, 0, 0)) {
						dzv = -24;
					} else if (cm->isValidPosition(x + dxv, y + dyv, z - 32, ixd, iyd, izd, _flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 32, ixd, iyd, izd, _flags, 1, 0, 0)) {
						dzv = -32;
					}
				}
				ok = true;
				break;
			} else if (cm->isValidPosition(x + dxv, y + dyv, z + dzv + 8, ixd, iyd, izd, shapeFlags, 1, 0, 0)) {
				dzv += 8;
				ok = true;
				break;
			}
			dxv /= 2;
			dyv /= 2;
			dzv /= 2;
		}
		if (ok) break;
	}

	// Yes, i know, not entirely correct
	avatar->collideMove(x + dxv, y + dyv, z + dzv, false, true);

	if (GAME_IS_CRUSADER) {
		// Keep the camera on the avatar while we're quick-moving.
		CameraProcess::SetCameraProcess(new CameraProcess(x + dxv, y + dyv, z + dzv));
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
