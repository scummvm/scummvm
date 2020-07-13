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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(QuickAvatarMoverProcess)

ProcId QuickAvatarMoverProcess::_amp[6] = { 0, 0, 0, 0, 0, 0 };
bool QuickAvatarMoverProcess::_clipping = false;
bool QuickAvatarMoverProcess::_quarter = false;

QuickAvatarMoverProcess::QuickAvatarMoverProcess() : Process(1), _dx(0), _dy(0), _dz(0), _dir(0) {
}

QuickAvatarMoverProcess::QuickAvatarMoverProcess(int x, int y, int z, int dir) : Process(1),
		_dx(x), _dy(y), _dz(z), _dir(dir) {
	QuickAvatarMoverProcess::terminateMover(dir);
	assert(_dir < 6);
	_amp[_dir] = getPid();
}

QuickAvatarMoverProcess::~QuickAvatarMoverProcess() {
}

void QuickAvatarMoverProcess::run() {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		terminate();
		return;
	}

	MainActor *avatar = getMainActor();
	int32 x, y, z;
	avatar->getLocation(x, y, z);
	int32 ixd, iyd, izd;
	avatar->getFootpadWorld(ixd, iyd, izd);

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	int32 dxv = this->_dx;
	int32 dyv = this->_dy;
	int32 dzv = this->_dz;

	for (int j = 0; j < 3; j++) {
		dxv = this->_dx;
		dyv = this->_dy;
		dzv = this->_dz;

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
	_amp[_dir] = 0;
}

void QuickAvatarMoverProcess::terminateMover(int dir) {
	assert(dir < 6);

	Kernel *kernel = Kernel::get_instance();

	QuickAvatarMoverProcess *p =
	    dynamic_cast<QuickAvatarMoverProcess *>(kernel->getProcess(_amp[dir]));

	if (p && !p->is_terminated())
		p->terminate();
}

void QuickAvatarMoverProcess::startMover(int x, int y, int z, int dir) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	if (! g->isAvatarInStasis()) {
		Process *p = new QuickAvatarMoverProcess(x, y, z, dir);
		Kernel::get_instance()->addProcess(p);
	} else {
		pout << "Can't quickmove: avatarInStasis" << Std::endl;
	}
}

void QuickAvatarMoverProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_dir);
	// don't save more information. We plan to terminate upon load
}

bool QuickAvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	// small safety precaution
	_dir = rs->readUint32LE();
	if (_dir < 6)
		_amp[_dir] = 0;
	else
		return false;

	terminateDeferred(); // Don't allow this process to continue
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
