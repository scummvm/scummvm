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

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(TargetReticleProcess)

TargetReticleProcess::TargetReticleProcess() : Process(), _reticleEnabled(true),
	_lastUpdate(0), _reticleSpriteProcess(0), _lastTargetDir(0x10), _lastTargetItem(0) {
}

void TargetReticleProcess::run() {
	Kernel *kernel = Kernel::get_instance();
	assert(kernel);
	uint32 frameno = kernel->getFrameNum();
	Process *spriteProc = nullptr;
	if (_reticleSpriteProcess != 0) {
		spriteProc = kernel->getProcess(_reticleSpriteProcess);
	}

	if (!_reticleEnabled) {
		if (spriteProc) {
			spriteProc->terminate();
		}
		_reticleSpriteProcess = 0;
		return;
	}

	if (frameno - _lastUpdate < 60) {
		return;
	}

	Item *item = findTargetItem();
	if (item && item->getObjId() != _lastTargetItem) {
		if (spriteProc)
			spriteProc->terminate();
		putTargetReticleOnItem(item);
	} else if (!item) {
		debug("New reticle target: NONE");
		if (spriteProc)
			spriteProc->terminate();
		_reticleSpriteProcess = 0;
		_lastTargetItem = 0;
		_lastTargetDir = 0x10;
	}
	// else, already targeting the right thing. do nothing.

	_lastUpdate = frameno;
}

Item *TargetReticleProcess::findTargetItem() {
	MainActor *mainactor = getMainActor();
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();

	if (!mainactor || !currentmap)
		return nullptr;

	int dir = mainactor->getDir();

	int32 x, y, z;
	mainactor->getCentre(x, y, z);

	Item *item = currentmap->findBestTargetItem(x, y, dir);
	return item;
}

void TargetReticleProcess::putTargetReticleOnItem(Item *item) {
	int32 x, y, z;

	// TODO: the game does a bunch of other maths here to pick the right location.
	// This is an over-simplification.
	item->getCentre(x, y, z);

	Process *p = new SpriteProcess(0x59a, 0, 5, 1, 10, x, y, z, false);

	_reticleSpriteProcess = Kernel::get_instance()->addProcess(p);
	_lastTargetItem = item->getObjId();
	debug("New reticle target: %d (%d, %d, %d)", _lastTargetItem, x, y, z);
}

void TargetReticleProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_lastUpdate);
	ws->writeUint16LE(_reticleSpriteProcess);
	ws->writeUint16LE(_lastTargetItem);
}

bool TargetReticleProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_lastUpdate = rs->readUint32LE();
	_reticleSpriteProcess = rs->readUint16LE();
	_lastTargetItem = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
