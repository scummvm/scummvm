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

#include "common/textconsole.h"

#include "tesoro/core/animbank.h"
#include "tesoro/core/levelnumber.h"
#include "tesoro/core/namekey.h"
#include "tesoro/kinds/inventoryitem.h"
#include "tesoro/world.h"

namespace Tesoro {

// The state bytes each subtype keeps.
static byte stateSizeOf(int subtype) {
	switch (subtype) {
	case 10: case 15: case 26: case 28: case 29: case 33: case 36: case 37: case 44:
		return 1;
	case 16:
		return 3;
	case 19:
		return 2;
	default:
		return 0;
	}
}

static const SpeechTable::Line *phraseAt(World &world, const ObjectRecord &record, uint offset, bool firstIsI) {
	char name[13];
	for (uint i = 0; i < 12; ++i)
		name[i] = (char)record.dataByte(offset + i);
	name[12] = '\0';
	if (firstIsI)
		name[0] = 'I';
	return name[0] ? world.phrase(name) : nullptr;
}

void InventoryItem::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	_world = &world;
	// The data's second byte is the item's number and also what picks the subtype.
	_subtype = record.dataByte(1);
	initAnimated(record, roomSortOffset, levelPlayers, stateSizeOf(_subtype));

	bool image = false;
	if (_anim.bankCount() > 0) {
		AnimPlayer &player = _anim.current();
		image = player.bank() && player.bank()->step(player.current()).hasImage();
	}
	_flags = ((_flags & ~kVisible) | 0x508 | (image ? kVisible : 0));

	_itemNumber = record.dataByte(1);
	_kind = _itemNumber;
	if (_anim.bankCount() > 0) {
		const byte first = record.dataByte(0);
		_anim.current().setRange(first, first, false, false, now);
	}
	_approachX = _x;
	_approachY = _y;
	// The level data's first letter of the name is always an I: the items of the interface carry that mark.
	_hoverName = phraseAt(world, record, 2, true);

	static const int order[kVerbs] = { 0, 1, 3, 4, 5, 2, 6 };
	for (int i = 0; i < 6; ++i)
		_verbLine[order[i]] = phraseAt(world, record, 0xe + 12 * i, false);

	const uint count = record.dataByte(0x62) | (record.dataByte(0x63) << 8);
	for (uint i = 0; i < count; ++i) {
		char key[13];
		for (uint c = 0; c < 12; ++c)
			key[c] = (char)record.dataByte(0x7c + 0x18 * i + c);
		key[12] = '\0';
		_comboKeys.push_back(nameKey64(key));
		_comboLines.push_back(phraseAt(world, record, 0x7c + 0x18 * i + 0xc, false));
	}
	_sortValue += 2;
	_flags &= ~kActive;
}

void InventoryItem::setItemActive(int level, const char *name, bool active) {
	if (Object *object = _world->findObject(level, name))
		object->setActive(active);
	else
		warning("Tesoro: no object %s", name);
}

int InventoryItem::runWith(const char *block, const char *label, byte *variables) {
	// An item's scripts act in the level Jim is in (their characters are looked up there), not in the interface level.
	return _world->startScriptInJimLevel(block, label, variables);
}

int InventoryItem::run(const char *block, const char *label, bool withState) {
	return runWith(block, label, withState ? state() : nullptr);
}

void InventoryItem::setActive(bool active) {
	if (!active) {
		if (_world->pointerItem() == this) {
			_world->cursorFrame(0, 0);
			_flags |= kVisible;
			_world->setPointerItem(nullptr);
		}
	} else {
		if (_world->interfaceCameraX() > 640) {
			if (InventoryItem *carried = _world->pointerItem())
				carried->_flags |= kVisible;
			_flags &= ~kVisible;
		}
		_world->setPointerItem(this);
		_world->cursorFrame(_itemNumber + 7, _itemNumber + 7);
	}
	_flags = (_flags & ~kActive) | (active ? kActive : 0);
}

int InventoryItem::verb(int verb) {
	byte flag = 0;
	switch (verb) {
	case 0:
		return InteractiveObject::verb(0);
	case 1:
		switch (_subtype) {
		case 16:
			run("FIPLATO", "mirar", true);
			return 1;
		case 19:
			state()[0] = _world->jimLevel() == _world->levelAt(kKitchen) && _world->isActive(kKitchen, "DICK");
			if (state()[0] != 0)
				_world->hideInventory();
			run("FIRECET", "mirar", true);
			return 1;
		case 28:
			run("FIMAPA", "mirar", true);
			return 1;
		case 44:
			run("FIPROBERV", "mirar", true);
			return 1;
		default:
			return 0;
		}
	case 4:
		switch (_subtype) {
		case 9:
			if (_world->jimStep() == 6 && _world->isActive(kInterface, "ILIBRO")) {
				Object *book = _world->findObject(kInterface, "ILIBRO");
				if (book && !book->stateBlob().empty() && book->stateBlob()[0] != 0) {
					_world->setJimStep(7);
					flag = 1;
				}
			}
			runWith("FICOFRE", "abrir", &flag);
			return 1;
		case 10:
			run("FICIRUELA", "abrir", true);
			return 1;
		case 15:
			state()[0] = 1;
			return 1;
		case 19:
			return this->verb(1);
		case 29:
			if (!_world->isActive(kInterface, "IHORQUILLA"))
				run("FIBOLSO", "abrir", false);
			return 1;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

int InventoryItem::useItem(int item) {
	byte flag = 0;
	byte *s = state();
	switch (_subtype) {
	case 16: {
		if (!_world->isActive(kInterface, "IRECETARIO"))
			return _world->startScriptInJimLevel("FNOUSAROBJ", "main", nullptr), 1;
		switch (item) {
		case 8:
			s[1] |= 4;
			run("FIPLATO", "polvora", true);
			break;
		case 10:
			flag = s[1] & 1;
			if ((s[1] & 1) == 0)
				setItemActive(kInterface, "ICIRUELA", false);
			s[1] |= 1;
			runWith("FICIRABR", "iplato", &flag);
			break;
		case 11:
			s[1] |= 8;
			run("FIPLATO", "pimientos", true);
			break;
		case 12:
			s[1] |= 2;
			run("FIPLATO", "revitalizante", true);
			break;
		case 13:
			flag = s[1] & 1;
			s[1] |= 1;
			runWith("FICIRABR", "iplato", &flag);
			setItemActive(kInterface, "ICIRUELABRT", false);
			break;
		case 20:
			s[1] |= 0x10;
			setItemActive(kInterface, "IPESCAO", false);
			run("FIPESCAO", "iplato", false);
			break;
		default:
			return 0;
		}
		if (s[1] == 0x1f) {
			s[0] = 2;
			return 1;
		}
		if (s[1] != 0)
			s[0] = 1;
		return 1;
	}
	case 28:
		if (item != 0x29 || _world->isActive(kMap, "RISCO"))
			return 0;
		s[0] = 1;
		setItemActive(kMap, "CUEVA", true);
		setItemActive(kMap, "RISCO", true);
		if (Object *map = _world->findObject(kMap, "MAPA"))
			_world->playFrames(*map, 1, 1);
		run("FIMAPA", "mechero", true);
		return 1;
	case 33:
		if (item != 25)
			return 0;
		setItemActive(kInterface, "IFERNAN", false);
		setItemActive(kInterface, "ICUBOPANT", false);
		setItemActive(kInterface, "ICUBOBARRO", true);
		return 1;
	case 36:
		if (item != 31)
			return 0;
		setItemActive(kInterface, "ICUBO", false);
		setItemActive(kInterface, "IRAMARESINA", false);
		setItemActive(kInterface, "ICUBOTAPON", true);
		return 1;
	case 37: {
		if (item != 0xff)
			return 0;
		Object *orzoguei = _world->findObject(kVillage, "ORZOGUEI");
		if (orzoguei && !orzoguei->stateBlob().empty() && orzoguei->stateBlob()[1] != 0 && _world->jimLevel() == _world->levelAt(kVillage) && s[0] == 0) {
			flag = 1;
			s[0] = 1;
			if (Object *again = _world->findObject(kVillage, "ORZOGUEI"))
				again->stateBlob()[0xc] = 1;
		}
		runWith("FICUB0MAR", "jim", &flag);
		return 1;
	}
	case 42:
		if (item != 22)
			return 0;
		setItemActive(kInterface, "ICUERDA2", false);
		setItemActive(kInterface, "ISOGA", false);
		setItemActive(kInterface, "IEXCUERDA", true);
		return 1;
	default:
		return 0;
	}
}

int InventoryItem::dispatch(int verb) {
	switch (verb) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		return this->verb(verb);
	case 5:
	case 6:
		return 0;
	default:
		break;
	}
	InventoryItem *held = _world->pointerItem();
	int result = held ? held->useItem(_itemNumber) : 0;
	if (result != 0)
		return 1;
	return useItem(held ? held->itemNumber() : 0);
}

int InventoryItem::update() {
	if (!_pending || _world->jimSpeaking())
		return 0;
	_world->setPointerActive(true);
	_pending = false;
	_world->removeFromMainList(this);
	return dispatch(_verb);
}

int InventoryItem::sayCombination(const SpeechTable::Line *line) {
	if (!line) {
		warning("Tesoro: an item combination without a line");
		return 0;
	}
	return answer(line);
}

int InventoryItem::setVerbAndClick(int verb) {
	_verb = verb;
	if (verb < 7 && verb >= 0 && _verbLine[verb])
		return answer(_verbLine[verb]);
	switch (verb) {
	case 0:
		setActive(true);
		return 3;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		if (this->verb(verb) != 0)
			return 1;
		setActive(true);
		return 3;
	default:
		break;
	}
	InventoryItem *held = _world->pointerItem();
	if (!held) {
		warning("Tesoro: an item used on another with nothing carried");
		return 0;
	}
	for (uint i = 0; i < _comboKeys.size(); ++i) {
		if (_world->findObjectByKey(kInterface, _comboKeys[i]) == held) {
			sayCombination(_comboLines[i]);
			_world->hideInventory();
			return 3;
		}
	}
	for (uint i = 0; i < held->_comboKeys.size(); ++i) {
		if (_world->findObjectByKey(kInterface, held->_comboKeys[i]) == this) {
			sayCombination(held->_comboLines[i]);
			_world->hideInventory();
			return 3;
		}
	}
	if (useItem(verb - 7) != 0 || held->useItem(_itemNumber) != 0)
		return 1;
	return answer(defaultLine("MODELO2"));
}

} // End of namespace Tesoro
