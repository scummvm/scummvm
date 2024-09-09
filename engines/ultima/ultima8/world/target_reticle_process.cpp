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


#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/message_box_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

TargetReticleProcess *TargetReticleProcess::_instance = nullptr;

DEFINE_RUNTIME_CLASSTYPE_CODE(TargetReticleProcess)

TargetReticleProcess::TargetReticleProcess() : Process(), _reticleEnabled(true),
		_lastUpdate(0), _reticleSpriteProcess(0), _lastTargetDir(dir_current), _lastTargetItem(0),
		_reticleStyle(0) {
	_instance = this;
	_type = 1; // persistent
	if (GAME_IS_REGRET)
		_reticleStyle = 3;
}

TargetReticleProcess::~TargetReticleProcess() {
	if (_instance == this)
		_instance = nullptr;
}

void TargetReticleProcess::run() {
	Kernel *kernel = Kernel::get_instance();
	assert(kernel);
	uint32 frameno = kernel->getFrameNum();
	Actor *mainactor = getControlledActor();
	Process *spriteProc = nullptr;
	if (_reticleSpriteProcess != 0) {
		spriteProc = kernel->getProcess(_reticleSpriteProcess);
	}

	if (!_reticleEnabled || (mainactor && !mainactor->isInCombat()) || !mainactor) {
		// Reticle not enabled, actor not in combat, or actor is gone.
		if (spriteProc) {
			spriteProc->terminate();
		}
		_reticleSpriteProcess = 0;
		return;
	}

	if (_reticleSpriteProcess && (!spriteProc || spriteProc->is_terminated())) {
		// The sprite proc has finished but the target is still valid - replace
		// with one that just holds the last frame.
		Item *target = getItem(_lastTargetItem);
		if (target)
			putTargetReticleOnItem(target, true);
	}

	if (frameno - _lastUpdate < 2 * Kernel::FRAMES_PER_SECOND) {
		return;
	}

	bool changed = findTargetItem();
	if (spriteProc && changed) {
		// Terminate the old process.
		spriteProc->terminate();
	}
	_lastUpdate = frameno;
}

bool TargetReticleProcess::findTargetItem() {
	Actor *mainactor = getControlledActor();
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	bool changed = false;

	if (!mainactor || !currentmap)
		return false;

	Direction dir = mainactor->getDir();

	Point3 pt = mainactor->getLocation();

	Item *item = currentmap->findBestTargetItem(pt.x, pt.y, pt.z, dir, dirmode_16dirs);

	if (item && item->getObjId() != _lastTargetItem) {
		Item *lastItem = getItem(_lastTargetItem);
		if (lastItem)
			lastItem->clearExtFlag(Item::EXT_TARGET);
		putTargetReticleOnItem(item, false);
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

void TargetReticleProcess::putTargetReticleOnItem(Item *item, bool only_last_frame) {
	// TODO: the game does a bunch of other maths here to pick the right location.
	// This is an over-simplification and is usually too high so it's
	// hacked a little lower.
	Point3 pt = item->getCentre();
	pt.z -= 8;

	Process *p;
	const int first_frame = _reticleStyle * 6;
	const int last_frame = first_frame + 5;
	if (!only_last_frame)
		p = new SpriteProcess(0x59a, first_frame, last_frame, 1, 10, pt.x, pt.y, pt.z, false);
	else
		p = new SpriteProcess(0x59a, last_frame, last_frame, 1, 1000, pt.x, pt.y, pt.z, false);

	_reticleSpriteProcess = Kernel::get_instance()->addProcess(p);
	_lastTargetItem = item->getObjId();
	item->setExtFlag(Item::EXT_TARGET);
	debug("New reticle target: %d (%d, %d, %d)", _lastTargetItem, pt.x, pt.y, pt.z);
}

void TargetReticleProcess::itemMoved(Item *item) {
	assert(item);
	if (!_reticleSpriteProcess || item->getObjId() != _lastTargetItem) {
		clearSprite();
		return;
	}

	Point3 pt = item->getCentre();

	Actor *mainactor = getControlledActor();
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
			_reticleSpriteProcess = 0;
			clearSprite();
		} else {
			spriteproc->move(pt.x, pt.y, pt.z);
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
	MessageBoxGump::Show("", msg, TEX32_PACK_RGB(0x70, 0x70, 0x70));
	setEnabled(newstate);
}

void TargetReticleProcess::toggleReticleStyle() {
	if (GAME_IS_REMORSE) {
		_reticleStyle = 0;
		return;
	}

	_reticleStyle++;
	if (_reticleStyle >= 4)
		_reticleStyle = 0;
}

void TargetReticleProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeByte(_reticleEnabled ? 1 : 0);
	ws->writeUint32LE(_lastUpdate);
	ws->writeUint16LE(_reticleSpriteProcess);
	ws->writeByte(_lastTargetDir);
	ws->writeUint16LE(_lastTargetItem);
	if (GAME_IS_REGRET)
		ws->writeUint16LE(_reticleStyle);
}

bool TargetReticleProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_reticleEnabled = (rs->readByte() != 0);
	_lastUpdate = rs->readUint32LE();
	_reticleSpriteProcess = rs->readUint16LE();
	_lastTargetDir = static_cast<Direction>(rs->readByte());
	_lastTargetItem = rs->readUint16LE();
	if (GAME_IS_REGRET)
		_reticleStyle = rs->readUint16LE();

	_type = 1; // should be persistent but older savegames may not know that.

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
