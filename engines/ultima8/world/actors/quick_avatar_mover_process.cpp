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

#include "ultima8/misc/pent_include.h"
#include "ultima8/world/actors/quick_avatar_mover_process.h"

#include "ultima8/world/actors/main_actor.h"
#include "ultima8/world/world.h"
#include "ultima8/world/current_map.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/kernel/gui_app.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/world/get_object.h"
#include "avatar_mover_process.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(QuickAvatarMoverProcess, Process);

ProcId QuickAvatarMoverProcess::amp[6] = { 0, 0, 0, 0, 0, 0 };
bool QuickAvatarMoverProcess::clipping = false;
bool QuickAvatarMoverProcess::quarter = false;

QuickAvatarMoverProcess::QuickAvatarMoverProcess() : Process(1), dx(0), dy(0), dz(0), dir(0) {
}

QuickAvatarMoverProcess::QuickAvatarMoverProcess(int x, int y, int z, int _dir) : Process(1), dx(x), dy(y), dz(z), dir(_dir) {
	QuickAvatarMoverProcess::terminateMover(dir);
	assert(dir < 6);
	amp[dir] = getPid();
}

QuickAvatarMoverProcess::~QuickAvatarMoverProcess() {
}

void QuickAvatarMoverProcess::run() {
	if (GUIApp::get_instance()->isAvatarInStasis()) {
		terminate();
		return;
	}

	MainActor *avatar = getMainActor();
	int32 x, y, z;
	avatar->getLocation(x, y, z);
	int32 ixd, iyd, izd;
	avatar->getFootpadWorld(ixd, iyd, izd);

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	int32 dx = this->dx;
	int32 dy = this->dy;
	int32 dz = this->dz;

	for (int j = 0; j < 3; j++) {
		dx = this->dx;
		dy = this->dy;
		dz = this->dz;

		if (j == 1) dx = 0;
		else if (j == 2) dy = 0;

		if (quarter) {
			dx /= 4;
			dy /= 4;
			dz /= 4;
		}

		bool ok = false;

		while (dx || dy || dz) {
			uint32 flags = avatar->getShapeInfo()->flags;

			if (!clipping || cm->isValidPosition(x + dx, y + dy, z + dz, ixd, iyd, izd, flags, 1, 0, 0)) {
				if (clipping && !dz) {
					if (cm->isValidPosition(x + dx, y + dy, z - 8, ixd, iyd, izd, flags, 1, 0, 0) &&
					        !cm->isValidPosition(x, y, z - 8, ixd, iyd, izd, flags, 1, 0, 0)) {
						dz = -8;
					} else if (cm->isValidPosition(x + dx, y + dy, z - 16, ixd, iyd, izd, flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 16, ixd, iyd, izd, flags, 1, 0, 0)) {
						dz = -16;
					} else if (cm->isValidPosition(x + dx, y + dy, z - 24, ixd, iyd, izd, flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 24, ixd, iyd, izd, flags, 1, 0, 0)) {
						dz = -24;
					} else if (cm->isValidPosition(x + dx, y + dy, z - 32, ixd, iyd, izd, flags, 1, 0, 0) &&
					           !cm->isValidPosition(x, y, z - 32, ixd, iyd, izd, flags, 1, 0, 0)) {
						dz = -32;
					}
				}
				ok = true;
				break;
			} else if (cm->isValidPosition(x + dx, y + dy, z + dz + 8, ixd, iyd, izd, flags, 1, 0, 0)) {
				dz += 8;
				ok = true;
				break;
			}
			dx /= 2;
			dy /= 2;
			dz /= 2;
		}
		if (ok) break;
	}

	// Yes, i know, not entirely correct
	avatar->collideMove(x + dx, y + dy, z + dz, false, true);


	// Prevent avatar from running an idle animation while moving around
	GUIApp::get_instance()->getAvatarMoverProcess()->resetIdleTime();
}

void QuickAvatarMoverProcess::terminate() {
	Process::terminate();
	amp[dir] = 0;
}

void QuickAvatarMoverProcess::terminateMover(int _dir) {
	assert(_dir < 6);

	Kernel *kernel = Kernel::get_instance();

	QuickAvatarMoverProcess *p =
	    p_dynamic_cast<QuickAvatarMoverProcess *>(kernel->getProcess(amp[_dir]));

	if (p && !p->is_terminated())
		p->terminate();
}

void QuickAvatarMoverProcess::startMover(int x, int y, int z, int _dir) {
	GUIApp *g = GUIApp::get_instance();
	if (! g->isAvatarInStasis()) {
		Process *p = new QuickAvatarMoverProcess(x, y, z, _dir);
		Kernel::get_instance()->addProcess(p);
	} else {
		pout << "Can't: avatarInStasis" << std::endl;
	}
}

void QuickAvatarMoverProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(dir);
	// don't save more information. We plan to terminate upon load
}

bool QuickAvatarMoverProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	// small safety precaution
	dir = ids->read4();
	if (dir < 6)
		amp[dir] = 0;
	else
		return false;

	terminateDeferred(); // Don't allow this process to continue
	return true;
}

void QuickAvatarMoverProcess::ConCmd_startMoveUp(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(-64, -64, 0, 0);
}

void QuickAvatarMoverProcess::ConCmd_startMoveDown(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(+64, +64, 0, 1);
}

void QuickAvatarMoverProcess::ConCmd_startMoveLeft(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(-64, +64, 0, 2);
}

void QuickAvatarMoverProcess::ConCmd_startMoveRight(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(+64, -64, 0, 3);
}

void QuickAvatarMoverProcess::ConCmd_startAscend(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(0, 0, 8, 4);
}

void QuickAvatarMoverProcess::ConCmd_startDescend(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::startMover(0, 0, -8, 5);
}

void QuickAvatarMoverProcess::ConCmd_stopMoveUp(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(0);
}

void QuickAvatarMoverProcess::ConCmd_stopMoveDown(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(1);
}

void QuickAvatarMoverProcess::ConCmd_stopMoveLeft(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(2);
}

void QuickAvatarMoverProcess::ConCmd_stopMoveRight(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(3);
}

void QuickAvatarMoverProcess::ConCmd_stopAscend(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(4);
}

void QuickAvatarMoverProcess::ConCmd_stopDescend(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::terminateMover(5);
}

void QuickAvatarMoverProcess::ConCmd_toggleQuarterSpeed(const Console::ArgvType &argv) {
	QuickAvatarMoverProcess::setQuarterSpeed(!QuickAvatarMoverProcess::isQuarterSpeed());
}

void QuickAvatarMoverProcess::ConCmd_toggleClipping(const Console::ArgvType &argv) {
	if (!GUIApp::get_instance()->areCheatsEnabled()) return;
	QuickAvatarMoverProcess::toggleClipping();
	pout << "QuickAvatarMoverProcess::clipping = " << QuickAvatarMoverProcess::isClipping() << std::endl;
}

} // End of namespace Ultima8
