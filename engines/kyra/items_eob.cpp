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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_EOB

#include "kyra/eobcommon.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

namespace Kyra {

void EobCoreEngine::loadItemDefs() {
	Common::SeekableReadStream *s = _res->createReadStream("item.dat");
	_numItems = s->readUint16LE();

	for (int i = 0; i < 600; i++)
		_items[i].block = -1;

	for (int i = 0; i < _numItems; i++) {
		_items[i].nameUnid = s->readByte();
		_items[i].nameId = s->readByte();
		_items[i].flags = s->readByte();
		_items[i].icon = s->readSByte();
		_items[i].type = s->readSByte();
		_items[i].pos = s->readSByte();
		_items[i].block = s->readSint16LE();
		_items[i].next = s->readSint16LE();
		_items[i].prev = s->readSint16LE();
		_items[i].level = s->readSByte();
		_items[i].value = s->readSByte();
	}

	_numItemNames = s->readUint16LE();
	for (int i = 0; i < _numItemNames; i++)
		s->read(_itemNames[i], 35);

	delete s;

	s = _res->createReadStream("itemtype.dat");
	uint16 numTypes = s->readUint16LE();

	_itemTypes = new EobItemType[numTypes];
	memset(_itemTypes, 0, sizeof(EobItemType) * numTypes);

	for (int i = 0; i < numTypes; i++) {
		_itemTypes[i].invFlags = s->readUint16LE();
		_itemTypes[i].handFlags = s->readUint16LE();
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
		_itemTypes[i].extraProperties = s->readUint16LE();
	}

	delete s;
}

Kyra::Item EobCoreEngine::duplicateItem(Item itemIndex) {
	EobItem *itm = &_items[itemIndex];

	if (itm->block == -1)
		return 0;

	Item i = 1;
	bool foundSlot = false;

	for (; i < 600; i++) {
		if (_items[i].block == -1) {
			foundSlot = true;
			break;
		}
	}

	if (!foundSlot)
		return 0;

	memcpy(&_items[i], &_items[itemIndex], sizeof(EobItem));
	return i;
}

void EobCoreEngine::setItemPosition(Item *itemQueue, int block, Item item, int pos) {
	if (!item)
		return;

	EobItem *itm = &_items[item];
	itm->pos = pos;
	itm->block = block;
	itm->level = block < 0 ? 0xff : _currentLevel;

	if (!*itemQueue) {
		*itemQueue = itm->next = itm->prev = item;
	} else {
		EobItem *itmQ = &_items[*itemQueue];
		EobItem *itmQN = &_items[itmQ->next];
		itm->prev = itmQN->prev;
		itm->next = itmQ->next;
		*itemQueue = itmQN->prev = itmQ->next = item;
	}
}

void EobCoreEngine::createInventoryItem(EobCharacter *c, Item itemIndex, int itemValue, int preferedInventorySlot) {
	if (itemIndex <= 0)
		return;

	itemIndex = duplicateItem(itemIndex);
	_items[itemIndex].flags |= 0x40;

	if (itemValue != -1)
		_items[itemIndex].value = itemValue;

	if (itemValue && ((_itemTypes[_items[itemIndex].type].extraProperties & 0x7f) < 4))
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

int EobCoreEngine::deleteInventoryItem(int charIndex, int slot) {
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
	}

	return _items[itm].value;
}

void EobCoreEngine::deleteBlockItem(uint16 block, int type) {
	uint16 itm = _levelBlockProperties[block].drawObjects;
	if (!itm)
		return;	
	
	for (uint16 i2 = itm, i = 0; itm != i2 || !i; i++ ) {
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
			setItemPosition((Item*)&_levelBlockProperties[block].drawObjects, block, i3, _items[i3].pos);
		}
	}
}

int EobCoreEngine::validateInventorySlotForItem(Item item, int charIndex, int slot) {
	if (item < 0)
		return 0;

	int offset = (_flags.gameID == GI_EOB1) ? 11 : (_flags.lang == Common::DE_DEU ? 16 : 12);

	if (slot == 17 && item && !itemUsableByCharacter(charIndex, item)) {
		_txt->printMessage(_itemExtraStrings[offset], -1, _characters[charIndex].name);
		return 0;
	}

	int itm = _characters[charIndex].inventory[slot];
	int ex = _itemTypes[_items[itm].type].extraProperties & 0x7f;

	if (slot < 2 && _items[itm].flags & 0x20 && ex > 0 && ex < 4) {
		_txt->printMessage(_itemExtraStrings[offset + 1], -1, _characters[charIndex].name);
		return 0;
	}

	uint16 v = item ? _itemTypes[_items[item].type].invFlags : 0xffff;
	if (v & _slotValidationFlags[slot])
		return 1;

	_txt->printMessage(_itemExtraStrings[offset + (_flags.gameID == GI_EOB1 ? 1 : 2)]);
	return 0;
}

void EobCoreEngine::deletePartyItem(Item itemType, int16 itemValue) {
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;

		EobCharacter *c = &_characters[i];
		int slot = checkCharacterInventoryForItem(i, itemType, itemValue);

		if (slot == -1)
			continue;

		int itm = c->inventory[slot];
		_items[itm].block = -1;
		c->inventory[slot] = 0;

		if (_currentControlMode == 0 && slot < 2 && i < 5)
			gui_drawWeaponSlot(i, slot);

		if (_currentControlMode == 1 && i == _updateCharNum)
			gui_drawInventoryItem(slot, 1, 0);
	}
}

int EobCoreEngine::itemUsableByCharacter(int charIndex, Item item) {
	if (!item)
		return 1;

	return (_itemTypes[_items[item].type].allowedClasses & _classModifierFlags[_characters[charIndex].cClass]);
}

int EobCoreEngine::countQueuedItems(Item itemQueue, int16 id, int16 type, int count, int includeFlyingItems) {
	uint16 o1 = itemQueue;
	uint16 o2 = o1;
	
	if (!o1)
		return 0;

	int res = 0;
		
	for (bool forceLoop = true; o1 != o2 || forceLoop; o1 = _items[o1].prev) {
		EobItem *itm = &_items[o1];
		forceLoop = false;
		if (id != -1 || type != -1) {
			if ((id != -1 || (id == -1 && type != itm->type)) && (type != -1) || (id != o1))
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

int EobCoreEngine::getQueuedItem(Item *items, int pos, int id) {
	Item o1 = *items;
	Item o2 = o1;

	if (!o1)
		return 0;

	EobItem *itm = &_items[o1];
	
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

void EobCoreEngine::printFullItemName(Item item) {
	EobItem *itm = &_items[item];
	const char *nameUnid = _itemNames[itm->nameUnid];
	const char *nameId = _itemNames[itm->nameId];
	uint8 f = _itemTypes[itm->type].extraProperties & 0x7f;
	int8 v = itm->value;
	
	const char *tstr2 = 0;
	const char *tstr3 = 0;
	int e = 0;

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
					tmpString = Common::String::format(_itemExtraStrings[3], v, nameUnid);
				else
					tmpString = Common::String::format(_itemExtraStrings[4], v, nameUnid);
				break;

			case 9:
				tstr2 = _itemExtraStrings[5];
				tstr3 = _spells[v].name;
				e = 1;
				break;

			case 10:
				tstr2 = _itemExtraStrings[6];
				tstr3 = _spells[_flags.gameID == GI_EOB1 ? (_clericSpellOffset + v) : v].name;
				e = 1;
				break;

			case 14:
				tstr2 = _itemExtraStrings[8];
				tstr3 = _itemSuffixStrings[8];
				break;

			case 16:
				tstr2 = _itemExtraStrings[7];
				tstr3 = _itemSuffixStrings[v + 6];
				e = 0;
				break;

			case 18:
				if (v == 5) {
					tstr2 = _itemExtraStrings[_flags.lang == Common::EN_ANY ? 9 : 10];
					e = 1;
				} else {
					tstr2 = _itemExtraStrings[9];
					e = 0;
				}
				tstr3 = _itemSuffixStrings[v + (_flags.lang == Common::EN_ANY ? 11 : 15)];
				break;

			default:
				tmpString = nameUnid;
				break;
		}
	

		if (tstr3) {
			if (!tstr2) {
				tmpString = Common::String::format(_itemExtraStrings[_flags.lang == Common::EN_ANY ? 10 : 11], tstr3);
			} else {
				if (e == 1) {
					if (tstr2 == _itemExtraStrings[12])
						tmpString = Common::String::format(_itemExtraStrings[_flags.lang == Common::EN_ANY ? 11 : 14], tstr2, tstr3);
					else
						tmpString = Common::String::format(_itemExtraStrings[_flags.gameID == GI_EOB1 ? 10 : (_flags.lang == Common::EN_ANY ? 11 : 13)], tstr2, tstr3);
				} else {
					tmpString = Common::String::format(_itemExtraStrings[_flags.gameID == GI_EOB1 ? 10 : (_flags.lang == Common::EN_ANY ? 11 : 15)], tstr2, tstr3);
				}
			}
		}
	} else {
		tmpString = (itm->flags & 0x40) ? nameId : nameUnid;
	}

	_txt->printMessage(tmpString.c_str());
}

void EobCoreEngine::identifyQueuedItems(Item itemQueue) {
	if (!itemQueue)
		return;

	Item first = itemQueue;
	do {
		_items[itemQueue].flags |= 0x40;
		itemQueue = _items[itemQueue].prev;

	} while (first != itemQueue);
}

void EobCoreEngine::drawItemIconShape(int pageNum, Item itemId, int x, int y) {
	int icn = _items[itemId].icon;
	bool applyBluePal = ((_partyEffectFlags & 2) && (_items[itemId].flags & 0x80)) ? true : false;

	if (applyBluePal) {
		_screen->setFadeTableIndex(3);
		_screen->setShapeFadeMode(1, true);
	}

	_screen->drawShape(pageNum, _itemIconShapes[icn], x, y, 0);

	if (applyBluePal) {
		_screen->setFadeTableIndex(4);
		_screen->setShapeFadeMode(1, false);
	}
}

bool EobCoreEngine::isMagicWeapon(Item itemIndex) {
	return (_items[itemIndex].type > 10 && _items[itemIndex].type < 18);
}

bool EobCoreEngine::checkInventoryForRings(int charIndex, int itemValue) {
	for (int i = 25; i <= 26; i++) {
		int itm = _characters[charIndex].inventory[i];
		if (itm && _items[itm].type == 47 && _items[itm].value == itemValue)
			return true;
	}
	return false;
}

void EobCoreEngine::eatItemInHand(int charIndex) {
	EobCharacter *c = &_characters[charIndex];
	if (!testCharacter(charIndex, 5)) {
		_txt->printMessage(_warningStrings[1], -1, c->name);
	} else if (_itemInHand && _items[_itemInHand].type != 31) {
		_txt->printMessage(_warningStrings[3]);
	} else if (_items[_itemInHand].value == -1) {
		_txt->printMessage(_warningStrings[2]);
		snd_playSoundEffect(79);
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

bool EobCoreEngine::launchObject(int charIndex, Item item, uint16 startBlock, int startPos, int dir, int type) {
	EobFlyingObject *t = _flyingObjects;
	int slot = 0;
	for (; slot < 10; slot++) {
		if (!t->enable)
			break;
		t++;
	}

	if (slot == 10)
		return false;

	setItemPosition((Item*)&_levelBlockProperties[startBlock].drawObjects, startBlock, item, startPos | 4);

	t->enable = 1;
	t->u2 = 1;
	t->flags = 0;
	t->direction = dir;
	t->distance = 12;
	t->curBlock = startBlock;
	t->curPos = startPos;
	t->item = item;
	t->objectType = type;
	t->attackerId = charIndex;
	t->callBackIndex = 0;

	snd_playSoundEffect(type == 7 ? 26 : 11);	
	return true;
}

void EobCoreEngine::launchMagicObject(int charIndex, int type, uint16 startBlock, int startPos, int dir) {
	EobFlyingObject *t = _flyingObjects;
	int slot = 0;
	for (; slot < 10; slot++) {
		if (!t->enable)
			break;
		t++;
	}

	if (slot == 10)
		return;

	t->enable = 2;
	t->u2 = 1;
	t->flags = _magicFlightObjectProperties[(type << 2) + 2];
	t->direction = dir;
	t->distance = _magicFlightObjectProperties[(type << 2) + 1];
	t->curBlock = startBlock;
	t->curPos = startPos;
	t->item = type;
	t->objectType = _magicFlightObjectProperties[(type << 2) + 3];
	t->attackerId = charIndex;
	t->u2 = 1;
	t->callBackIndex = _magicFlightObjectProperties[type << 2];
	_sceneUpdateRequired = true;
}

bool EobCoreEngine::updateObjectFlight(EobFlyingObject *fo, int block, int pos) {
	uint8 wallFlags = _wllWallFlags[_levelBlockProperties[block].walls[fo->direction ^ 2]];
	if (fo->enable == 1) {
		if ((wallFlags & 1) || (fo->u2) || ((wallFlags & 2) && (_dscItemShapeMap[_items[fo->item].icon] >= 15))) {
			getQueuedItem((Item*)&_levelBlockProperties[fo->curBlock].drawObjects, 0, fo->item);
			setItemPosition((Item*)&_levelBlockProperties[block].drawObjects, block, fo->item, pos | 4);
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

bool EobCoreEngine::updateFlyingObjectHitTest(EobFlyingObject *fo, int block, int pos) {
	if (fo->u2 && (fo->curBlock != _currentBlock || fo->attackerId >= 0) && (!blockHasMonsters(block) || fo->attackerId < 0))
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

void EobCoreEngine::explodeObject(EobFlyingObject *fo, int block, Item item) {
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

	if (b == 0 || (b == 1 && (fo->direction & 1) == (_currentDirection & 1))) {
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

void EobCoreEngine::endObjectFlight(EobFlyingObject *fo) {
	if (fo->enable == 1) {
		_items[fo->item].pos &= 3;
		runLevelScript(fo->curBlock, 4);
		updateEnvironmentalSfx(18);
	}
	memset(fo, 0, sizeof(EobFlyingObject));
}

void EobCoreEngine::checkFlyingObjects() {
	for (int i = 0; i < 10; i++) {
		EobFlyingObject *fo = &_flyingObjects[i];
		if (!fo->enable)
			continue;
		if (updateFlyingObjectHitTest(fo, fo->curBlock, fo->curPos))
			endObjectFlight(fo);
	}
}

void EobCoreEngine::reloadWeaponSlot(int charIndex, int slotIndex, int itemType, int arrowOrDagger) {
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

