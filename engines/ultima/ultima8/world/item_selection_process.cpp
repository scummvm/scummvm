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


#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/item_selection_process.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/usecode/uc_list.h"

namespace Ultima {
namespace Ultima8 {

ItemSelectionProcess *ItemSelectionProcess::_instance = nullptr;

static const uint32 SELECTOR_SHAPE = 0x5a3;
static const uint16 SELECT_FAILED_SFX_REMORSE = 0xb0;
static const uint16 SELECT_FAILED_SFX_REGRET = 0x1a7;

DEFINE_RUNTIME_CLASSTYPE_CODE(ItemSelectionProcess)

ItemSelectionProcess::ItemSelectionProcess() : Process(), _selectedItem(0),
_ax(0), _ay(0), _az(0) {
	_instance = this;
	_type = 1; // persistent
}

ItemSelectionProcess::~ItemSelectionProcess() {
	if (_instance == this)
		_instance = nullptr;
}

void ItemSelectionProcess::run() {
}

bool ItemSelectionProcess::selectNextItem(bool grab) {
	MainActor *mainactor = getMainActor();
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();

	if (!mainactor || !currentmap)
		return false;

	Point3 pt = mainactor->getCentre();
	_ax = pt.x;
	_ay = pt.y;
	_az = mainactor->getZ();

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	currentmap->areaSearch(&uclist, script, sizeof(script),
						   mainactor, 0x200, false);

	Std::vector<Item *> candidates;

	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		ObjId itemid = uclist.getuint16(i);
		Item *item = getItem(itemid);
		if (!item)
			continue;

		const ShapeInfo *info = item->getShapeInfo();

		// Maybe this can be done with a loopscript,
		// but this is how the game does it..
		if (item->hasFlags(Item::FLG_HANGING))
			continue;
		uint16 family = item->getFamily();
		if (item->getShape() == 0x4ed || family == ShapeInfo::SF_CRUWEAPON ||
			family == ShapeInfo::SF_CRUAMMO || family == ShapeInfo::SF_CRUBOMB ||
			family == ShapeInfo::SF_CRUINVITEM ||
			(info && (info->_flags & ShapeInfo::SI_CRU_SELECTABLE))) {

			Point3 c = item->getCentre();
			int32 iz = item->getZ();
			if (abs(c.x - _ax) > 0x100 || abs(c.y - _ay) > 0x100 ||
				(iz - _az) >= 0x50 || (_az - iz) >= 0x18)
				continue;

			candidates.push_back(item);
			if (grab) {
				const ShapeInfo *info_g = item->getShapeInfo();
				if (!info_g || !(info_g->_flags & ShapeInfo::SI_CRU_SELECTABLE)) {
					MainActor *actor = getMainActor();
					if (actor)
						actor->addItemCru(item, true);
				}
			}
		}
	}

	if (grab) {
		clearSelection();
		return false;
	}

	if (candidates.size() < 1) {
		AudioProcess *audio = AudioProcess::get_instance();
		assert(audio);
		// Play the "beeboop" selection failed sound.
		uint16 sfxno = GAME_IS_REGRET ? SELECT_FAILED_SFX_REGRET : SELECT_FAILED_SFX_REMORSE;
		if (!audio->isSFXPlaying(sfxno))
			audio->playSFX(sfxno, 0x10, 0, 1);
		clearSelection();
		return false;
	}
	Item *item = candidates[0];
	if (_selectedItem) {
		// Pick the next item
		int offset = 0;
		for (Std::vector<Item *>::const_iterator iter = candidates.begin();
			 iter != candidates.end();
			 offset++, iter++) {
			ObjId num = (*iter)->getObjId();
			if (_selectedItem == num) {
				offset++;
				break;
			}
		}
		offset = offset % candidates.size();
		item = candidates[offset];
	}

	putItemSelectionOnItem(item);
	return true;
}

void ItemSelectionProcess::useSelectedItem() {
	if (!_selectedItem)
		return;
	Item *item = getItem(_selectedItem);
	if (item) {
		const ShapeInfo *info = item->getShapeInfo();
		if (info && (info->_flags & ShapeInfo::SI_CRU_SELECTABLE)) {
			item->callUsecodeEvent_use();
		} else {
			MainActor *actor = getMainActor();
			if (actor)
				actor->addItemCru(item, true);
		}
	}
	clearSelection();
}

void ItemSelectionProcess::avatarMoved() {
	if (!_selectedItem)
		return;
	Item *item = getItem(_selectedItem);
	MainActor *mainactor = getMainActor();

	// Only clear if actor has moved a little
	if (item && mainactor) {
		Point3 a = mainactor->getCentre();
		uint32 range = MAX(abs(a.x - _ax), MAX(abs(a.y - _ay), abs(a.z - _az)));
		if (range > 16)
			clearSelection();
	}
}

void ItemSelectionProcess::clearSelection() {
	if (!getItemNum())
		return;
	Item *sprite = getItem(getItemNum());
	if (sprite) {
		sprite->destroy();
	}
	setItemNum(0);
	_selectedItem = 0;
}

void ItemSelectionProcess::putItemSelectionOnItem(Item *item) {
	assert(item);

	clearSelection();

	Point3 pt = item->getCentre();
	_selectedItem = item->getObjId();

	Item *sprite = ItemFactory::createItem(SELECTOR_SHAPE, 0, 0, Item::FLG_DISPOSABLE,
										   0, 0, Item::EXT_SPRITE, true);
	sprite->move(pt);
	setItemNum(sprite->getObjId());
}

void ItemSelectionProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint16LE(_selectedItem);
	ws->writeSint32LE(_ax);
	ws->writeSint32LE(_ay);
	ws->writeSint32LE(_az);
}

bool ItemSelectionProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_selectedItem = rs->readUint16LE();
	_ax = rs->readSint32LE();
	_ay = rs->readSint32LE();
	_az = rs->readSint32LE();
	_type = 1; // should be persistent but older savegames may not know that.
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
