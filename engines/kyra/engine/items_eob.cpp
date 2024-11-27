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

#ifdef ENABLE_EOB

#include "kyra/engine/eobcommon.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/sound.h"

namespace Kyra {

Common::SeekableReadStreamEndian *EoBCoreEngine::getItemDefinitionFile(int index) {
	assert(index == 0 || index == 1);
	return _res->createEndianAwareReadStream(index ? "itemtype.dat" : "item.dat");
}

void EoBCoreEngine::loadItemDefs() {
	Common::SeekableReadStreamEndian *s = getItemDefinitionFile(0);
	_items.clear();
	_numItems = s->readUint16();

	for (int i = 0; i < _numItems; i++) {
		EoBItem it;
		it.nameUnid = s->readByte();
		it.nameId = s->readByte();
		it.flags = s->readByte();
		it.icon = s->readSByte();
		it.type = s->readSByte();
		it.pos = s->readSByte();
		it.block = s->readSint16();
		it.next = s->readSint16();
		it.prev = s->readSint16();
		it.level = s->readByte();
		it.value = s->readSByte();
		_items.push_back(it);
	}

	if (_flags.platform == Common::kPlatformSegaCD) {
		int temp = 0;
		const uint8 *pos = _staticres->loadRawData(kEoB1MapLevelData, temp);

		for (int i = _numItems; i < _numItems + temp / 14; i++) {
			EoBItem it;
			it.nameUnid = *pos++;
			it.nameId = *pos++;
			it.flags = *pos++;
			it.icon = (int8)*pos++;
			it.type = (int8)*pos++;
			it.pos = (int8)*pos++;
			it.block = (int16)READ_BE_UINT16(pos);
			pos += 2;
			it.next = (int16)READ_BE_UINT16(pos);
			pos += 2;
			it.prev = (int16)READ_BE_UINT16(pos);
			pos += 2;
			it.level = *pos++;
			it.value = (int8)*pos++;
			_items.push_back(it);
		}
		_numItems += (temp / 14);
		_items[22].nameUnid = _items[27].nameUnid = _items[28].nameUnid = _items[29].nameUnid = _items[59].nameUnid = 96;

		for (int i = _numItems; i < 500; i++)
			_items.emplace_back(EoBItem());

		_items[498].block = _items[499].block = -2;
	}

	if (_itemNamesStatic) {
		_numItemNames = _numItemNamesStatic;
		for (int i = 0; i < _numItemNames; i++) {
			assert(strlen(_itemNamesStatic[i]) < 35);
			Common::strlcpy(_itemNames[i], _itemNamesStatic[i], 34);
		}
	} else {
		_numItemNames = s->readUint16();
		for (int i = 0; i < _numItemNames; i++)
			s->read(_itemNames[i], 35);
	}

	delete s;

	s = getItemDefinitionFile(1);
	uint16 numTypes = s->readUint16();

	delete[] _itemTypes;
	size_t itemTypeTableSize = 65;
	_itemTypes = new EoBItemType[itemTypeTableSize]();

	for (int i = 0; i < numTypes; i++) {
		_itemTypes[i].invFlags = s->readUint16();
		_itemTypes[i].handFlags = s->readUint16();
		_itemTypes[i].armorClass = s->readSByte();
		_itemTypes[i].allowedClasses = s->readSByte();
		_itemTypes[i].requiredHands = s->readSByte();
		_itemTypes[i].dmgNumDiceS = s->readSByte();
		_itemTypes[i].dmgNumPipsS = s->readSByte();
		_itemTypes[i].dmgIncS = s->readSByte();
		_itemTypes[i].dmgNumDiceL = s->readSByte();
		_itemTypes[i].dmgNumPipsL = s->readSByte();
		_itemTypes[i].dmgIncL = s->readSByte();
		_itemTypes[i].unk1 = s->readByte();
		_itemTypes[i].extraProperties = s->readUint16();
	}

	/*
	 * In the original Eye of the Beholder 1 data files, bow and sling item
	 * types have damage dice that are not in line with AD&D 2nd edition rules:
	 *
	 * Bow:   dmgNumPipsS = 8, dmgNumPipsL = 10
	 * Sling: dmgNumPipsS = 6, dmgNumPipsL = 6
	 */
	if (_flags.gameID == GI_EOB1 && _configADDRuleEnhancements) {
		debugC(1, kDebugLevelMain, "patching EotB 1 bow   (%d), old dice S d%d, L d%d, new dice S d6, L d6",
			   kItemTypeBow, _itemTypes[kItemTypeBow].dmgNumPipsS, _itemTypes[kItemTypeBow].dmgNumPipsL);
		_itemTypes[kItemTypeBow].dmgNumPipsS = 6;
		_itemTypes[kItemTypeBow].dmgNumPipsL = 6;
		debugC(1, kDebugLevelMain, "patching EotB 1 sling (%d), old dice S d%d, L  d%d, new dice S d4, L d4",
			   kItemTypeSling, _itemTypes[kItemTypeSling].dmgNumPipsS, _itemTypes[kItemTypeSling].dmgNumPipsL);
		_itemTypes[kItemTypeSling].dmgNumPipsS = 4;
		_itemTypes[kItemTypeSling].dmgNumPipsL = 4;
	}

	delete s;
}

Kyra::Item EoBCoreEngine::duplicateItem(Item itemIndex) {
	assert(itemIndex < (Item)_items.size());
	EoBItem *itm = &_items[itemIndex];

	if (itm->block == -1)
		return 0;

	Item i = 1;
	bool foundSlot = false;

	for (; i < (Item)_items.size(); i++) {
		if (_items[i].block == -1) {
			foundSlot = true;
			break;
		}
	}

	if (!foundSlot)
		_items.push_back(*itm);
	else
		_items[i] = *itm;

	return i;
}

Item EoBCoreEngine::createItemOnCurrentBlock(Item itemIndex) {
	Item itm = duplicateItem(itemIndex);
	setItemPosition((Item *)&_levelBlockProperties[_currentBlock].drawObjects, _currentBlock, itm, _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);
	return itm;
}

void EoBCoreEngine::setItemPosition(Item *itemQueue, int block, Item item, int pos) {
	if (!item)
		return;

	EoBItem *itm = &_items[item];
	itm->pos = pos;
	itm->block = block;
	itm->level = block < 0 ? 0xFF : _currentLevel;

	if (!*itemQueue) {
		*itemQueue = itm->next = itm->prev = item;
	} else {
		EoBItem *itmQ = &_items[*itemQueue];
		EoBItem *itmQN = &_items[itmQ->next];
		itm->prev = itmQN->prev;
		itm->next = itmQ->next;
		*itemQueue = itmQN->prev = itmQ->next = item;
	}
}

void EoBCoreEngine::createInventoryItem(EoBCharacter *c, Item itemIndex, int16 itemValue, int preferedInventorySlot) {
	if (itemIndex <= 0)
		return;

	itemIndex = duplicateItem(itemIndex);
	_items[itemIndex].flags |= 0x40;

	if (itemValue != -1)
		_items[itemIndex].value = itemValue;

	if (itemValue && ((_itemTypes[_items[itemIndex].type].extraProperties & 0x7F) < 4))
		_items[itemIndex].flags |= 0x80;

	if (c->inventory[preferedInventorySlot]) {
		for (int i = 2; i < 16; i++) {
			if (!c->inventory[i]) {
				c->inventory[i] = itemIndex;
				return;
			}
		}
	} else {
		c->inventory[preferedInventorySlot] = itemIndex;
	}
}

int EoBCoreEngine::deleteInventoryItem(int charIndex, int slot) {
	int itm = (slot == -1) ? _itemInHand : _characters[charIndex].inventory[slot];
	_items[itm].block = -1;

	if (slot == -1) {
		setHandItem(0);
	} else {
		_characters[charIndex].inventory[slot] = 0;

		if (_currentControlMode == 1)
			gui_drawInventoryItem(slot, 1, 0);

		if (_currentControlMode == 0)
			gui_drawCharPortraitWithStats(charIndex);

		_screen->updateScreen();
	}

	return _items[itm].value;
}

void EoBCoreEngine::deleteBlockItem(uint16 block, int type) {
	uint16 itm = _levelBlockProperties[block].drawObjects;
	if (!itm)
		return;

	_levelBlockProperties[block].drawObjects = 0;

	for (uint16 i2 = itm, i = 0; itm != i2 || !i; i++) {
		if (type == _items[itm].type || type == -1) {
			_items[itm].block = -1;
			_items[itm].level = 0;
			uint16 i3 = itm;
			itm = _items[itm].prev;
			_items[i3].prev = _items[i3].next = 0;
		} else {
			uint16 i3 = itm;
			itm = _items[itm].prev;
			_items[i3].prev = _items[i3].next = 0;
			setItemPosition((Item *)&_levelBlockProperties[block].drawObjects, block, i3, _items[i3].pos);
		}
	}
}

int EoBCoreEngine::validateInventorySlotForItem(Item item, int charIndex, int slot) {
	if (item < 0)
		return 0;

	if (slot == 27)
		return 1;

	if (slot == 17 && item && !itemUsableByCharacter(charIndex, item)) {
		_txt->printMessage(_validateArmorString[0], -1, _characters[charIndex].name, _itemNames[_items[item].nameUnid]);
		return 0;
	}

	int itm = _characters[charIndex].inventory[slot];
	int ex = _itemTypes[_items[itm].type].extraProperties & 0x7F;

	if (_items[itm].flags & 0x20 && (_flags.gameID == GI_EOB1 || slot < 2)) {
		if (_flags.gameID == GI_EOB2 && ex > 0 && ex < 4)
			_txt->printMessage(_validateCursedString[0], -1, _characters[charIndex].name, _itemNames[_items[item].nameUnid]);
		return 0;
	}

	uint16 v = item ? _itemTypes[_items[item].type].invFlags : 0xFFFF;

	// WORKAROUND: The game allows putting the 4 horns and the red rings from the ringmaster riddle into inventory ring slots.
	// This causes a graphics glitch, since these items are too large for that slot. I prevent this here. Patching it while
	// loading up items.dat and itemtypes.dat would be preferable but seems too complicated (if not impossible), since the inv
	// flags are part of itemtypes.dat and it might be wrong to patch these flags for the whole item type.
	if (_flags.gameID == GI_EOB2 && (_items[item].icon == 107 || _items[item].icon == 61))
		v &= ~0x100;

	if (v & _slotValidationFlags[slot])
		return 1;

	_txt->printMessage(_validateNoDropString[0]);
	return 0;
}

int EoBCoreEngine::stripPartyItems(int16 itemType, int16 itemValue, int handleValueMode, int numItems) {
	int itemsLeft = numItems;

	for (bool runloop = true; runloop && itemsLeft;) {
		runloop = false;
		for (int i = 0; i < 6 && itemsLeft; i++) {
			if (!testCharacter(i, 1))
				continue;

			for (int ii = 0; ii < 27 && itemsLeft; ii++) {
				if (ii == 16)
					continue;

				Item itm = _characters[i].inventory[ii];
				if ((_items[itm].type == itemType) && ((handleValueMode == -1 && _items[itm].value <= itemValue) || (handleValueMode == 0 && _items[itm].value == itemValue) || (handleValueMode == 1 && _items[itm].value >= itemValue))) {
					_characters[i].inventory[ii] = 0;
					_items[itm].block = -1;
					itemsLeft--;
					runloop = true;
				}
			}
		}
	}

	return numItems - itemsLeft;
}

bool EoBCoreEngine::deletePartyItems(int16 itemType, int16 itemValue) {
	bool res = false;
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;

		EoBCharacter *c = &_characters[i];
		for (int slot = checkInventoryForItem(i, itemType, itemValue); slot != -1; slot = checkInventoryForItem(i, itemType, itemValue)) {
			int itm = c->inventory[slot];
			_items[itm].block = -1;
			c->inventory[slot] = 0;
			res = true;

			if (!_dialogueField) {
				if (_currentControlMode == 0 && slot < 2 && i < 5)
					gui_drawWeaponSlot(i, slot);

				if (_currentControlMode == 1 && i == _updateCharNum)
					gui_drawInventoryItem(slot, 1, 0);
			}
		}
	}

	if (_itemInHand > 0) {
		if ((itemType == -1 || itemType == _items[_itemInHand].type) && (itemValue == -1 || itemValue == _items[_itemInHand].value)) {
			_items[_itemInHand].block = -1;
			setHandItem(0);
			res = true;
		}
	}

	_screen->updateScreen();

	return res;
}

int EoBCoreEngine::itemUsableByCharacter(int charIndex, Item item) const {
	if (!item)
		return 1;

	return (_itemTypes[_items[item].type].allowedClasses & _classModifierFlags[_characters[charIndex].cClass]);
}

int EoBCoreEngine::countQueuedItems(Item itemQueue, int16 id, int16 type, int count, int includeFlyingItems) const {
	uint16 o1 = itemQueue;
	uint16 o2 = o1;

	if (!o1)
		return 0;

	int res = 0;

	for (bool forceLoop = true; o1 != o2 || forceLoop; o1 = _items[o1].prev) {
		const EoBItem *itm = &_items[o1];
		forceLoop = false;
		if (id != -1 || type != -1) {
			if (((id != -1) || (id == -1 && type != itm->type)) && ((type != -1) || (type == -1 && id != o1)))
				continue;
		}

		if (!includeFlyingItems) {
			if (itm->pos > 3 && itm->pos < 8)
				continue;
		}

		if (!count)
			return o1;

		res++;
	}

	return res;
}

int EoBCoreEngine::getQueuedItem(Item *items, int pos, int id) {
	Item o1 = *items;
	Item o2 = o1;

	if (!o1)
		return 0;

	EoBItem *itm = &_items[o1];

	for (bool forceLoop = true; o1 != o2 || forceLoop; o1 = itm->prev) {
		itm = &_items[o1];
		forceLoop = false;
		if ((id != -1 || (id == -1 && itm->pos != pos)) && id != o1)
			continue;

		Item n = itm->next;
		Item p = itm->prev;
		_items[n].prev = p;
		_items[p].next = n;
		itm->next = itm->prev = itm->block = 0;
		itm->level = 0;
		if (o1 == *items)
			*items = p;
		if (o1 == *items)
			*items = 0;

		return o1;
	}

	return 0;
}

void EoBCoreEngine::printFullItemName(Item item) {
	EoBItem *itm = &_items[item];
	const char *nameUnid = _itemNames[itm->nameUnid];
	const char *nameId = _itemNames[itm->nameId];
	uint8 f = _itemTypes[itm->type].extraProperties & 0x7F;
	int8 v = itm->value;

	const char *tstr2 = 0;
	const char *tstr3 = 0;

	bool correctSuffixCase = false;

	Common::String tmpString;

	if ((itm->flags & 0x40) && !strlen(nameId)) {
		switch (f) {
		case 0:
		case 1:
		case 2:
		case 3:
			if (v == 0)
				tmpString = nameUnid;
			else if (v < 0)
				tmpString = _flags.gameID == GI_EOB1 ? Common::String::format(_cursedString[0], nameUnid, v) : Common::String::format(_cursedString[0], v, nameUnid);
			else
				tmpString = _flags.gameID == GI_EOB1 ? Common::String::format(_enchantedString[0], nameUnid, v) : Common::String::format(_enchantedString[0], v, nameUnid);
			break;

		case 9:
			tstr2 = _magicObjectStrings[0];
			tstr3 = _spells[v].name;
			correctSuffixCase = true;
			break;

		case 10:
			tstr2 = _magicObjectStrings[1];
			tstr3 = _spells[_flags.gameID == GI_EOB1 ? (_clericSpellOffset + v) : v].name;
			correctSuffixCase = true;
			break;

		case 14:
			tstr2 = _magicObjectStrings[3];
			if (_flags.gameID == GI_EOB1)
				v--;
			tstr3 = _suffixStringsPotions[v];
			break;

		case 16:
			tstr2 = _magicObjectStrings[2];
			tstr3 = _suffixStringsRings[v];
			break;

		case 18:
			if (_flags.gameID == GI_EOB2 && v == 5) {
				if (_flags.lang == Common::DE_DEU)
					tstr2 = _magicObjectString5[0];
				else
					tstr3 = _magicObjectString5[0];
				correctSuffixCase = true;
			} else {
				tstr2 = _magicObjectStrings[4];
			}
			tstr3 = _suffixStringsWands[v];
			break;

		default:
			tmpString = nameUnid;
			break;
		}

		if (tstr3) {
			if (!tstr2) {
				tmpString = tstr3;
			} else if (_flags.lang == Common::ZH_TWN) {
				if (!correctSuffixCase)
					SWAP(tstr2, tstr3);
				Common::String t2(tstr2);
				tmpString = Common::String::format(_patternSuffix[t2.contains(_patternGrFix1[0]) || t2.contains(_patternGrFix2[0]) ? 0 : 1], tstr2, tstr3);
			} else {
				if (_flags.lang == Common::JA_JPN)
					SWAP(tstr2, tstr3);
				if (correctSuffixCase) {
					if (tstr2 == _magicObjectString5[0])
						tmpString = Common::String::format(_patternGrFix2[0], tstr2, tstr3);
					else
						tmpString = Common::String::format(_patternGrFix1[0], tstr2, tstr3);
				} else {
					tmpString = Common::String::format(_patternSuffix[0], tstr2, tstr3);
				}
			}
		}
	} else {
		tmpString = (itm->flags & 0x40) ? nameId : nameUnid;
	}

	int cs = (_flags.platform == Common::kPlatformSegaCD && _flags.lang == Common::JA_JPN && _screen->getNumberOfCharacters((tmpString).c_str()) >= 17) ? _screen->setFontStyles(_screen->_currentFont, Font::kStyleNarrow2) : -1;

	_txt->printMessage(makeTwoByteString(tmpString).c_str());

	if (cs != -1)
		_screen->setFontStyles(_screen->_currentFont, cs);
}

void EoBCoreEngine::identifyQueuedItems(Item itemQueue) {
	if (!itemQueue)
		return;

	Item first = itemQueue;
	do {
		_items[itemQueue].flags |= 0x40;
		itemQueue = _items[itemQueue].prev;

	} while (first != itemQueue);
}

void EoBCoreEngine::drawItemIconShape(int pageNum, Item itemId, int x, int y) {
	int icn = _items[itemId].icon;
	bool applyBluePal = ((_partyEffectFlags & 2) && (_items[itemId].flags & 0x80)) ? true : false;
	const uint8 *ovl = 0;
	const uint8 *shp = _itemIconShapes[icn];

	if (_xtraItemIconShapes) {
		bool applyBluePalC = applyBluePal;
		applyBluePal = false;
		if (_items[itemId].nameUnid == 23)
			shp = _xtraItemIconShapes[0];
		else if (_items[itemId].nameUnid == 97)
			shp = _xtraItemIconShapes[1];
		else if (_items[itemId].nameId == 39)
			shp = _xtraItemIconShapes[2];
		else
			applyBluePal = applyBluePalC;
	}

	if (applyBluePal) {
		if (_blueItemIconShapes) {
			shp = _blueItemIconShapes[icn];
		} else if (_flags.gameID == GI_EOB1) {
			ovl = (_configRenderMode == Common::kRenderCGA) ? _itemsOverlayCGA : &_itemsOverlay[icn << 4];
		} else {
			_screen->setFadeTable(_lightBlueFadingTable);
			_screen->setShapeFadingLevel(1);
		}
	}

	_screen->drawShape(pageNum, shp, x, y, 0, ovl ? 2 : 0, ovl);

	if (applyBluePal) {
		_screen->setFadeTable(_greyFadingTable);
		_screen->setShapeFadingLevel(0);
	}
}

bool EoBCoreEngine::isMagicEffectItem(Item itemIndex) {
	return (itemIndex > 10 && itemIndex < 18);
}

bool EoBCoreEngine::checkInventoryForRings(int charIndex, int itemValue) {
	for (int i = 25; i <= 26; i++) {
		int itm = _characters[charIndex].inventory[i];
		if (itm && _items[itm].type == 47 && _items[itm].value == itemValue)
			return true;
	}
	return false;
}

void EoBCoreEngine::eatItemInHand(int charIndex) {
	EoBCharacter *c = &_characters[charIndex];
	if (!testCharacter(charIndex, 5)) {
		_txt->printMessage(_warningStrings[1], -1, c->name);
	} else if (_itemInHand && _items[_itemInHand].type != 31 && !(_flags.gameID == GI_EOB1 && _items[_itemInHand].type == 49)) {
		_txt->printMessage(_warningStrings[_flags.gameID == GI_EOB1 ? 2 : 3]);
	} else if (_items[_itemInHand].value == -1) {
		printWarning(_warningStrings[2]);
	} else {
		c->food += _items[_itemInHand].value;
		if (c->food > 100)
			c->food = 100;

		_items[_itemInHand].block = -1;
		setHandItem(0);
		gui_drawFoodStatusGraph(charIndex);
		_screen->updateScreen();
		snd_playSoundEffect(9);
	}
}

bool EoBCoreEngine::launchObject(int charIndex, Item item, uint16 startBlock, int startPos, int dir, int type, Item projectileWeapon) {
	EoBFlyingObject *t = _flyingObjects;
	int slot = 0;
	for (; slot < 10; slot++) {
		if (!t->enable)
			break;
		t++;
	}

	if (slot == 10)
		return false;

	setItemPosition((Item *)&_levelBlockProperties[startBlock].drawObjects, startBlock, item, startPos | 4);

	t->enable = 1;
	t->starting = 1;
	t->flags = 0;
	t->direction = dir;
	t->distance = 12;
	t->curBlock = startBlock;
	t->curPos = startPos;
	t->item = item;
	t->objectType = type;
	t->attackerId = charIndex;
	t->callBackIndex = 0;

	t->projectileWeapon = projectileWeapon;

	snd_playSoundEffect(type == 7 ? 26 : 11);
	return true;
}

void EoBCoreEngine::launchMagicObject(int charIndex, int type, uint16 startBlock, int startPos, int dir) {
	EoBFlyingObject *t = _flyingObjects;
	int slot = 0;
	for (; slot < 10; slot++) {
		if (!t->enable)
			break;
		t++;
	}

	if (slot == 10)
		return;

	t->enable = 2;
	t->starting = 1;
	t->flags = _magicFlightObjectProperties[(type << 2) + 2];
	t->direction = dir;
	t->distance = _magicFlightObjectProperties[(type << 2) + 1];
	t->curBlock = startBlock;
	t->curPos = startPos;
	t->item = type;
	t->objectType = _magicFlightObjectProperties[(type << 2) + 3];
	t->attackerId = charIndex;
	t->callBackIndex = _magicFlightObjectProperties[type << 2];
	_sceneUpdateRequired = true;
}

bool EoBCoreEngine::updateObjectFlight(EoBFlyingObject *fo, int block, int pos) {
	uint8 wallFlags = _wllWallFlags[_levelBlockProperties[block].walls[fo->direction ^ 2]];
	if (fo->enable == 1) {
		if ((wallFlags & 1) || (fo->starting) || ((wallFlags & 2) && (_dscItemShapeMap[_items[fo->item].icon] >= 15))) {
			getQueuedItem((Item *)&_levelBlockProperties[fo->curBlock].drawObjects, 0, fo->item);
			setItemPosition((Item *)&_levelBlockProperties[block].drawObjects, block, fo->item, pos | 4);
			fo->curBlock = block;
			fo->curPos = pos;
			fo->distance--;
			return true;

		} else {
			_clickedSpecialFlag = 0x10;
			specialWallAction(block, fo->direction);
			return false;
		}

	} else {
		if (!(wallFlags & 1) && (fo->curBlock != block))
			return false;
		fo->curBlock = block;
		fo->curPos = pos;
		if (fo->distance != 255)
			fo->distance--;
	}
	return true;
}

bool EoBCoreEngine::updateFlyingObjectHitTest(EoBFlyingObject *fo, int block, int pos) {
	if (fo->starting && (fo->curBlock != _currentBlock || fo->attackerId >= 0) && (!blockHasMonsters(fo->curBlock) || fo->attackerId < 0))
		return false;

	if (fo->enable == 2) {
		if (fo->callBackIndex)
			return (this->*_spells[fo->callBackIndex].endCallback)(fo);
	}

	if (blockHasMonsters(block)) {
		for (int i = 0; i < 30; i++) {
			if (!isMonsterOnPos(&_monsters[i], block, pos, 1))
				continue;
			if (flyingObjectMonsterHit(fo, i))
				return true;
		}

	} else if (block == _currentBlock) {
		return flyingObjectPartyHit(fo);
	}

	return false;
}

void EoBCoreEngine::explodeObject(EoBFlyingObject *fo, int block, Item item) {
	if (_partyResting) {
		snd_processEnvironmentalSoundEffect(35, _currentBlock);
		return;
	}

	const uint8 *table = (_expObjectTblIndex[item] == 0) ? _expObjectAnimTbl1 : ((_expObjectTblIndex[item] == 1) ? _expObjectAnimTbl2 : _expObjectAnimTbl3);
	int tableSize = (_expObjectTblIndex[item] == 0) ? _expObjectAnimTbl1Size : ((_expObjectTblIndex[item] == 1) ? _expObjectAnimTbl2Size : _expObjectAnimTbl3Size);

	int tl = 0;
	for (; tl < 18; tl++) {
		if (_visibleBlockIndex[tl] == block)
			break;
	}

	if (tl == 18)
		return;

	int b = _expObjectTlMode ? _expObjectTlMode[tl] : 2;

	uint8 fdr = fo ? fo->direction : 0;
	if (b == 0 || (b == 1 && (fdr & 1) == (_currentDirection & 1))) {
		snd_processEnvironmentalSoundEffect(35, _currentBlock);
		return;
	}

	uint8 dm = _dscDimMap[tl];
	int16 x1 = 0;
	int16 x2 = 0;

	setLevelShapesDim(tl, x1, x2, 5);

	if (x2 < x1)
		return;

	if (fo)
		fo->enable = 0;

	drawScene(1);

	if (fo)
		fo->enable = 2;

	_screen->fillRect(0, 0, 176, 120, 0, 2);
	uint8 col = _screen->getPagePixel(2, 0, 0);
	drawSceneShapes(_expObjectShpStart[dm]);

	setLevelShapesDim(tl, x1, x2, 5);
	_screen->updateScreen();

	_screen->setGfxParameters(_dscShapeCoords[(tl * 5 + 4) << 1] + 88, 48, col);
	snd_processEnvironmentalSoundEffect(35, _currentBlock);

	disableSysTimer(2);
	if (dm == 0) {
		_screen->drawExplosion(3, 147, 35, 20, 7, table, tableSize);
	} else if (dm == 1) {
		_screen->drawExplosion(2, 147, 35, 20, 7, table, tableSize);
	} else if (dm == 2) {
		_screen->drawExplosion(1, 147, 35, 20, 7, table, tableSize);
	} else if (dm == 3) {
		_screen->drawExplosion(0, 460, 50, 20, 4, table, tableSize);
	}
	enableSysTimer(2);
}

void EoBCoreEngine::endObjectFlight(EoBFlyingObject *fo) {
	if (fo->enable == 1) {
		_items[fo->item].pos &= 3;
		runLevelScript(fo->curBlock, 4);
		snd_updateEnvironmentalSfx(18);
	}
	memset(fo, 0, sizeof(EoBFlyingObject));
}

void EoBCoreEngine::checkFlyingObjects() {
	if (!_runFlag)
		return;

	for (int i = 0; i < 10; i++) {
		EoBFlyingObject *fo = &_flyingObjects[i];
		if (!fo->enable)
			continue;
		if (updateFlyingObjectHitTest(fo, fo->curBlock, fo->curPos))
			endObjectFlight(fo);
	}
}

void EoBCoreEngine::reloadWeaponSlot(int charIndex, int slotIndex, int itemType, int arrowOrDagger) {
	if (arrowOrDagger && _characters[charIndex].inventory[16]) {
		_characters[charIndex].inventory[slotIndex] = getQueuedItem(&_characters[charIndex].inventory[16], 0, -1);
	} else {
		for (int i = 24; i >= 22; i--) {
			if (!_characters[charIndex].inventory[i])
				continue;
			if (_items[_characters[charIndex].inventory[i]].type == itemType && itemType != -1)
				continue;
			_characters[charIndex].inventory[slotIndex] = _characters[charIndex].inventory[i];
			_characters[charIndex].inventory[i] = 0;
			return;
		}
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB
