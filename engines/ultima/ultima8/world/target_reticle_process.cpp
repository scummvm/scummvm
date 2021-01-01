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

#include "ultima/ultima8/gumps/message_box_gump.h"
#include "ultima/ultima8/games/game_data.h"
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

TargetReticleProcess *TargetReticleProcess::_instance = nullptr;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(TargetReticleProcess)

TargetReticleProcess::TargetReticleProcess() : Process(), _reticleEnabled(true),
		_lastUpdate(0), _reticleSpriteProcess(0), _lastTargetDir(dir_current), _lastTargetItem(0) {
	_instance = this;
}

void TargetReticleProcess::run() {
	Kernel *kernel = Kernel::get_instance();
	assert(kernel);
	uint32 frameno = kernel->getFrameNum();
	Actor *mainactor = getMainActor();
	Process *spriteProc = nullptr;
	if (_reticleSpriteProcess != 0) {
		spriteProc = kernel->getProcess(_reticleSpriteProcess);
	}

	if (!_reticleEnabled || (mainactor && !mainactor->isInCombat())) {
		if (spriteProc) {
			spriteProc->terminate();
		}
		_reticleSpriteProcess = 0;
		return;
	}

	if (frameno - _lastUpdate < 2 * Kernel::FRAMES_PER_SECOND) {
		return;
	}

	bool changed = findTargetItem();
	if (spriteProc && changed)
		// Terminate the old process.
		spriteProc->terminate();
	_lastUpdate = frameno;
}

bool TargetReticleProcess::findTargetItem() {
	MainActor *mainactor = getMainActor();
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	bool changed = false;

	if (!mainactor || !currentmap)
		return false;

	Direction dir = mainactor->getDir();

	int32 x, y, z;
	mainactor->getCentre(x, y, z);

	Item *item = currentmap->findBestTargetItem(x, y, dir, dirmode_16dirs);

	if (item && item->getObjId() != _lastTargetItem) {
		Item *lastItem = getItem(_lastTargetItem);
		if (lastItem)
			lastItem->clearExtFlag(Item::EXT_TARGET);
		putTargetReticleOnItem(item);
		_lastTargetDir = dir;
		changed = true;
	} else if (!item) {
		if (_lastTargetItem) {
			debug("New reticle target: NONE");
			Item *lastItem = getItem(_lastTargetItem);
			if (lastItem)
				lastItem->clearExtFlag(Item::EXT_TARGET);
		}
		clearSprite();
		changed = true;
	}
	// else, already targeting the right thing. do nothing.
	return changed;
}

void TargetReticleProcess::avatarMoved() {
	_lastUpdate = 0;
}

void TargetReticleProcess::putTargetReticleOnItem(Item *item) {
	int32 x, y, z;

	// TODO: the game does a bunch of other maths here to pick the right location.
	// This is an over-simplification and is usually too high so it's
	// hacked a little lower.
	item->getCentre(x, y, z);
	z -= 8;

	Process *p = new SpriteProcess(0x59a, 0, 5, 1, 10, x, y, z, false);

	_reticleSpriteProcess = Kernel::get_instance()->addProcess(p);
	_lastTargetItem = item->getObjId();
	item->setExtFlag(Item::EXT_TARGET);
	debug("New reticle target: %d (%d, %d, %d)", _lastTargetItem, x, y, z);
}

void TargetReticleProcess::itemMoved(Item *item) {
	assert(item);
	if (!_reticleSpriteProcess || item->getObjId() != _lastTargetItem) {
		clearSprite();
		return;
	}

	int32 x, y, z;
	item->getCentre(x, y, z);

	MainActor *mainactor = getMainActor();
	int actordir = -1;
	int dirtoitem = -2;
	if (mainactor) {
		actordir = mainactor->getDir();
		dirtoitem = mainactor->getDirToItemCentre(*item);
	}

	SpriteProcess *spriteproc = dynamic_cast<SpriteProcess *>(Kernel::get_instance()->getProcess(_reticleSpriteProcess));

	if (spriteproc) {
		if (actordir != _lastTargetDir || dirtoitem != _lastTargetDir) {
			spriteproc->terminate();
			clearSprite();
		} else {
			spriteproc->move(x, y, z);
		}
	}
}

void TargetReticleProcess::clearSprite() {
	_reticleSpriteProcess = 0;
	if (_lastTargetItem) {
		Item *item = getItem(_lastTargetItem);
		if (item) {
			item->clearExtFlag(Item::EXT_TARGET);
		}
	}
	_lastTargetItem = 0;
	_lastTargetDir = dir_current;
}

void TargetReticleProcess::toggle() {
	bool newstate = !getEnabled();
	Std::string msg = newstate ? _TL_("TARGETING RETICLE ACTIVE") : _TL_("TARGETING RETICLE INACTIVE");
	MessageBoxGump::Show("", msg, 0xFF707070);
	setEnabled(newstate);
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
