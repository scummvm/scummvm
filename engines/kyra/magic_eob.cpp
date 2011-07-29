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

namespace Kyra {

void EobCoreEngine::useMagicBookOrSymbol(int charIndex, int type) {
	EobCharacter *c = &_characters[charIndex];
	_openBookSpellLevel = c->slotStatus[3];
	_openBookSpellSelectedItem = c->slotStatus[2];
	_openBookSpellListOffset = c->slotStatus[4];
	_openBookChar = charIndex;
	_openBookType = type;
	_openBookSpellList = (type == 1) ? _clericSpellList : _mageSpellList;
	_openBookAvailableSpells = (type == 1) ? c->clericSpells : c->mageSpells;
	int8 *tmp = _openBookAvailableSpells + _openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem;

	if (*tmp <= 0) {
		for (bool loop = true; loop && _openBookSpellSelectedItem < 10; ) {
			tmp = _openBookAvailableSpells + _openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem;
			if (*tmp > 0) {
				if (_openBookSpellSelectedItem > 5) {
					_openBookSpellListOffset = 6;
					_openBookSpellSelectedItem -= 6;
				}
				loop = false;
			} else {
				_openBookSpellSelectedItem++;
			}
		}

		if (_openBookSpellSelectedItem == 10) {
			_openBookSpellListOffset = 0;
			_openBookSpellSelectedItem = 6;
		}
	}

	if (!_updateFlags)
		_screen->copyRegion(64, 121, 0, 0, 112, 56, 0, 10, Screen::CR_NO_P_CHECK);
	_updateFlags = 1;
	gui_setPlayFieldButtons();
	gui_drawSpellbook();
}

void EobCoreEngine::useMagicScroll(int charIndex, int type, int weaponSlot) {
	_openBookCharBackup = _openBookChar;
	_openBookTypeBackup = _openBookType;
	_castScrollSlot = weaponSlot + 1;
	_openBookChar = charIndex;
	_openBookType = type <= _clericSpellOffset ? 0 : 1;
	castSpell(type, weaponSlot);
}

void EobCoreEngine::usePotion(int charIndex, int weaponSlot) {
	EobCharacter *c = &_characters[_openBookChar];

	int val = deleteInventoryItem(charIndex, weaponSlot);
	snd_playSoundEffect(10);

	if (_flags.gameID == GI_EOB1)
		val--;

	switch (val) {
	case 0:
		sparkEffectDefensive(charIndex);
		c->strengthCur = 22;
		c->strengthExtCur = 0;
		setCharEventTimer(charIndex, 546 * rollDice(1, 4, 4), 7, 1);
		break;

	case 1:
		sparkEffectDefensive(charIndex);
		modifyCharacterHitpoints(charIndex, rollDice(2, 4, 2));
		break;

	case 2:
		sparkEffectDefensive(charIndex);
		modifyCharacterHitpoints(charIndex, rollDice(3, 8, 3));
		break;

	case 3:
		statusAttack(charIndex, 2, _potionStrings[0], 0, 1, 8, 1);
		c->effectFlags &= ~0x2000;
		if (c->flags & 2)
			return;
		break;

	case 4:
		sparkEffectDefensive(charIndex);
		c->food = 100;
		if (_currentControlMode)
			gui_drawCharPortraitWithStats(charIndex);
		break;

	case 5:
		sparkEffectDefensive(charIndex);
		c->effectFlags |= 0x10000;
		setCharEventTimer(charIndex, 546 * rollDice(1, 4, 4), 12, 1);
		snd_playSoundEffect(100);
		gui_drawCharPortraitWithStats(charIndex);
		break;

	case 6:
		sparkEffectDefensive(charIndex);
		c->effectFlags |= 0x40;
		gui_drawCharPortraitWithStats(charIndex);
		break;

	case 7:
		sparkEffectDefensive(charIndex);
		neutralizePoison(charIndex);
		break;

	default:
		break;
	}

	_txt->printMessage(_potionStrings[1], -1, c->name, _potionEffectStrings[val]);
}

void EobCoreEngine::useWand(int charIndex, int weaponSlot) {
	int v = _items[_characters[charIndex].inventory[weaponSlot]].value - 1;
	if (!v) {
		_txt->printMessage(_wandStrings[0]);
		return;
	}

	if (v != 5)
		useMagicScroll(charIndex, _wandTypes[v], weaponSlot);
	else if (_flags.gameID == GI_EOB2)
		useMagicScroll(charIndex, 64, weaponSlot);
	else {
		uint16 bl1 = calcNewBlockPosition(_currentBlock, _currentDirection);
		uint16 bl2 = calcNewBlockPosition(bl1, _currentDirection);
		snd_playSoundEffect(98);
		sparkEffectOffensive();
		
		if ((_wllWallFlags[_levelBlockProperties[bl2].walls[_currentDirection ^ 2]] & 4) && !(_levelBlockProperties[bl2].flags & 7) && (_levelBlockProperties[bl1].flags & 7)) {
			for (int i = 0; i < 30; i++) {
				if (_monsters[i].block != bl1)
					continue;
				placeMonster(&_monsters[i], bl2, -1);
				_sceneUpdateRequired = true;
			}
		} else {
			_txt->printMessage(_wandStrings[1]);
		}
	}
}

void EobCoreEngine::castSpell(int spell, int weaponSlot) {
	EobSpell *s = &_spells[spell];
	EobCharacter *c = &_characters[_openBookChar];
	_activeSpell = spell;

	if ((s->flags & 0x100) && (c->effectFlags & 0x40))
		// remove invisibility effect
		removeCharacterEffect(10, _openBookChar, 1);

	int ci = _openBookChar;
	if (ci > 3)
		ci -= 2;

	_activeSpellCasterPos = _dropItemDirIndex[(_currentDirection << 2) + ci];

	if (s->flags & 0x400) {
		if (c->inventory[0] && c->inventory[1]) {
			printWarning(_magicStrings1[2]);
			return;
		}

		if (isMagicWeapon(c->inventory[0]) || isMagicWeapon(c->inventory[1])) {
			printWarning(_magicStrings1[3]);
			return;
		}
	}

	if (!(_flags.gameID == GI_EOB2 && _activeSpell == 62)) {
		if (!_castScrollSlot) {
			int8 tmp = _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem];
			if (_openBookSpellListOffset + _openBookSpellSelectedItem < 8)
				memmove(&_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem], &_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem + 1], 8 - (_openBookSpellListOffset + _openBookSpellSelectedItem));
			_openBookAvailableSpells[_openBookSpellLevel * 10 + 8] = -tmp;
			if (_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem] < 0) {
				if (--_openBookSpellSelectedItem == -1) {
					if (_openBookSpellListOffset) {
						_openBookSpellListOffset = 0;
						_openBookSpellSelectedItem = 5;
					} else {
						_openBookSpellSelectedItem = 6;
					}
				}
			}
		} else if (weaponSlot != -1) {
			updateUsedCharacterHandItem(_openBookChar, weaponSlot);
		}
	}

	_txt->printMessage(_magicStrings1[4], -1, c->name, s->name);

	if (s->flags & 0x20) {
		castOnWhomDialogue();
		return;
	}

	_activeSpellCaster = _openBookChar;
	startSpell(spell);
}

void EobCoreEngine::removeCharacterEffect(int spell, int charIndex, int showWarning) {
	assert(spell >= 0);
	EobCharacter *c = &_characters[charIndex];
	EobSpell *s = &_spells[spell];

	if (showWarning)
		printWarning(Common::String::format(_magicStrings3[_flags.gameID == GI_EOB1 ? 3 : 2], c->name, s->name).c_str());

	if (s->endCallback)
		(this->*s->endCallback)(0);

	if (s->flags & 1)
		c->effectFlags &= ~s->effectFlags;

	if (s->flags & 4)
		_partyEffectFlags &= ~s->effectFlags;

	if (s->flags & 0x200) {
		for (int i = 0; i < 6; i++) {
			if (!testCharacter(i, 1))
				continue;
			if (!testCharacter(i, 2) && !(s->flags & 0x800))
				continue;
			_characters[i].effectFlags &= ~s->effectFlags;
		}
	}

	if (s->flags & 0x2)
		recalcArmorClass(_activeSpellCaster);

	if (showWarning) {
		if (s->flags & 0x20A0)
			gui_drawCharPortraitWithStats(charIndex);
		else if (s->flags & 0x40)
			gui_drawAllCharPortraitsWithStats();
	}
}

void EobCoreEngine::removeAllCharacterEffects(int charIndex) {
	EobCharacter *c = &_characters[charIndex];
	memset(c->effectsRemainder, 0, 4);

	for (int i = 0; i < 10; i++) {
		if (c->events[i] < 0)
			removeCharacterEffect(-c->events[i], charIndex, 0);
		c->timers[i] = 0;
		c->events[i] = 0;
	}

	setupCharacterTimers();
	recalcArmorClass(charIndex);
	c->disabledSlots = 0;
	c->slotStatus[0] = c->slotStatus[1] = 0;
	c->damageTaken = 0;
	c->strengthCur = c->strengthMax;
	c->strengthExtCur = c->strengthExtMax;
	gui_drawAllCharPortraitsWithStats();
}

void EobCoreEngine::castOnWhomDialogue() {
	printWarning(_magicStrings3[0]);
	gui_setCastOnWhomButtons();
}

void EobCoreEngine::startSpell(int spell) {
	EobSpell *s = &_spells[spell];
	EobCharacter *c = &_characters[_activeSpellCaster];
	snd_playSoundEffect(s->sound);

	if (s->flags & 0xa0)
		sparkEffectDefensive(_activeSpellCaster);
	else if (s->flags & 0x40)
		sparkEffectDefensive(-1);
	else if (s->flags & 0x1000)
		sparkEffectOffensive();

	if (s->flags & 0x20) {
		_txt->printMessage(c->name);
		_txt->printMessage(_flags.gameID == GI_EOB1 ? _magicStrings3[1] : _magicStrings1[5]);
	}

	if ((s->flags & 0x30) && (s->effectFlags & c->effectFlags)) {
		printWarning(Common::String::format(_magicStrings7[0], c->name, s->name).c_str());
	} else if ((s->flags & 0x50) && (s->effectFlags & _partyEffectFlags)) {
		printWarning(Common::String::format(_magicStrings7[1], s->name).c_str());
	} else {
		if (s->flags & 8)
			setSpellEventTimer(spell, s->timingPara[0], s->timingPara[1], s->timingPara[2], s->timingPara[3]);

		_returnAfterSpellCallback = false;
		if (s->startCallback)
			(this->*s->startCallback)();
		if (_returnAfterSpellCallback)
			return;

		if (s->flags & 1)
			c->effectFlags |= s->effectFlags;
		if (s->flags & 4)
			_partyEffectFlags |= s->effectFlags;

		if (s->flags & 0x200) {
			for (int i = 0; i < 6; i++) {
				if (!testCharacter(i, 1))
					continue;
				if (!testCharacter(i, 2) && !(s->flags & 0x800))
					continue;
				_characters[i].effectFlags |= s->effectFlags;
			}
		}

		if (s->flags & 2)
			recalcArmorClass(_activeSpellCaster);

		if (s->flags & 0x20A0)
			gui_drawCharPortraitWithStats(_activeSpellCaster);
		if (s->flags & 0x40)
			gui_drawAllCharPortraitsWithStats();
	}

	if (_castScrollSlot) {
		gui_updateSlotAfterScrollUse();
	} else {
		c->disabledSlots |= 4;
		setCharEventTimer(_openBookChar, 72, 11, 1);
		gui_toggleButtons();
		gui_drawSpellbook();
	}

	if (_flags.gameID == GI_EOB2) {
		//_castSpellWd1 = spell;
		runLevelScript(_currentBlock, 0x800);
		//_castSpellWd1 = 0;
	}
}

void EobCoreEngine::sparkEffectDefensive(int charIndex) {
	int first = charIndex;
	int last = charIndex;
	if (charIndex == -1) {
		first = 0;
		last = 5;
	}

	for (int i = 0; i < 8; i++) {
		for (int ii = first; ii <= last; ii++) {
			if (!testCharacter(ii, 1) || (_currentControlMode && ii != _updateCharNum))
				continue;

			gui_drawCharPortraitWithStats(ii);

			for (int iii = 0; iii < 4; iii++) {
				int shpIndex = ((_sparkEffectDefSteps[i] & _sparkEffectDefSubSteps[iii]) >> _sparkEffectDefShift[iii]);
				if (!shpIndex)
					continue;
				int x = _sparkEffectDefAdd[iii * 2] - 8;
				int y = _sparkEffectDefAdd[iii * 2 + 1];
				if (_currentControlMode) {
					x += 181;
					y += 3;
				} else {
					x += (_sparkEffectDefX[ii] << 3);
					y += _sparkEffectDefY[ii];
				}
				_screen->drawShape(0, _sparkShapes[shpIndex - 1], x, y, 0);
				_screen->updateScreen();
			}
		}
		resetSkipFlag();
		delay(2 * _tickLength);
	}

	for (int i = first; i < last; i++)
		gui_drawCharPortraitWithStats(i);
}

void EobCoreEngine::sparkEffectOffensive() {
	disableSysTimer(2);
	_screen->copyRegion(0, 0, 0, 0, 176, 120, 0, 2, Screen::CR_NO_P_CHECK);

	for (int i = 0; i < 16; i++)
		_screen->copyRegionToBuffer(0, _sparkEffectOfX[i], _sparkEffectOfY[i], 16, 16, &_spellAnimBuffer[i << 8]);
	_screen->updateScreen();

	for (int i = 0; i < 11; i++) {
		for (int ii = 0; ii < 16; ii++)
			_screen->copyBlockToPage(2, _sparkEffectOfX[ii], _sparkEffectOfY[ii], 16, 16, &_spellAnimBuffer[ii << 8]);

		for (int ii = 0; ii < 16; ii++) {
			int shpIndex = (_sparkEffectOfFlags1[i] & _sparkEffectOfFlags2[ii]) >> _sparkEffectOfShift[ii];
			if (shpIndex)
				_screen->drawShape(2, _sparkShapes[shpIndex - 1], _sparkEffectOfX[ii], _sparkEffectOfY[ii], 0);
		}
		resetSkipFlag();
		delay(2 * _tickLength);
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	for (int i = 0; i < 16; i++)
		_screen->copyBlockToPage(0, _sparkEffectOfX[i], _sparkEffectOfY[i], 16, 16, &_spellAnimBuffer[i << 8]);

	_screen->updateScreen();
	enableSysTimer(2);
}

void EobCoreEngine::setSpellEventTimer(int spell, int timerBaseFactor, int timerLength, int timerLevelFactor, int updateExistingTimer) {
	int l = _openBookType == 1 ? getCharacterClericPaladinLevel(_openBookChar) : getCharacterMageLevel(_openBookChar);
	uint32 countdown = timerLength * timerBaseFactor + timerLength * l * timerLevelFactor;
	setCharEventTimer(_activeSpellCaster, countdown, -spell, updateExistingTimer);
}

void EobCoreEngine::sortCharacterSpellList(int charIndex) {
	int8 *list = _characters[charIndex].mageSpells;

	for (int i = 0; i < 16; ) {
		bool p = false;
		for (int ii = 0; ii < 9; ii++) {
			int8 *pos = &list[ii];

			int s1 = pos[0];
			int s2 = pos[1];

			if (s1 == 0)
				s1 = 80;
			else if (s1 < 0)
				s1 = s1 * -1 + 40;

			if (s2 == 0)
				s2 = 80;
			else if (s2 < 0)
				s2 = s2 * -1 + 40;

			if (s1 > s2) {
				SWAP(pos[0], pos[1]);
				p = true;
			}
		}

		if (p)
			continue;

		list += 10;
		if (++i == 8)
			list = _characters[charIndex].clericSpells;
	}
}

bool EobCoreEngine::magicObjectDamageHit(EobFlyingObject *fo, int dcTimes, int dcPips, int dcOffs, int level) {
	int ignoreAttackerId = fo->flags & 0x10;
	int singleTargetCheckAdjacent = fo->flags & 1;
	int blockDamage = fo->flags & 2;
	int hitTest = fo->flags & 4;

	int s = 5;
	int dmgType = 3;
	if (fo->flags & 8) {
		s = 4;
		dmgType = 0;
	}

	int dmgFlag = _spells[fo->callBackIndex].damageFlags;
	if (fo->attackerId >= 0)
		dmgFlag |= 0x800;

	bool res = false;
	if (!level)
		level = 1;

	if ((_levelBlockProperties[fo->curBlock].flags & 7) && (fo->attackerId >= 0 || ignoreAttackerId)) {
		_preventMonsterFlash = true;

		for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, blockDamage, singleTargetCheckAdjacent); *m != -1; m++) {
			int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;

			if (hitTest) {
				if (!characterAttackHitTest(fo->attackerId, *m, 0, 0))
					continue;
			}

			calcAndInflictMonsterDamage(&_monsters[*m], 0, 0, dmg, dmgFlag, s, dmgType);
			res = true;
		}
		updateAllMonsterShapes();

	} else if (fo->curBlock == _currentBlock && (fo->attackerId < 0 || ignoreAttackerId)) {
		if (blockDamage) {
			for (int i = 0; i < 6; i++) {
				if (!testCharacter(i, 1))
					continue;
				if (hitTest && !monsterAttackHitTest(&_monsters[0], i))
					continue;

				int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;
				res = true;

				calcAndInflictCharacterDamage(i, 0, 0, dmg, dmgFlag, s, dmgType);
			}
		} else {
			int c = _dscItemPosIndex[(_currentDirection << 2) + (fo->curPos & 3)];
			if ((c > 2) && (testCharacter(5, 1) || testCharacter(6, 1)) && rollDice(1, 2, -1))
				c += 2;

			if (!fo->item && (_characters[c].effectFlags & 8)) {
				res = true;
			} else {
				if ((_characters[c].flags & 1) && hitTest && !monsterAttackHitTest(&_monsters[0], c)) {
					int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;
					res = true;
					calcAndInflictCharacterDamage(c, 0, 0, dmg, dmgFlag, s, dmgType);
				}
			}
		}
	}

	if (res && (fo->flags & 0x40))
		explodeObject(fo, fo->curBlock, fo->item);
	else if ((_flags.gameID == GI_EOB1 && fo->item == 5) || (_flags.gameID == GI_EOB2 && fo->item == 4))
		res = false;

	return res;
}

bool EobCoreEngine::magicObjectStatusHit(EobMonsterInPlay *m, int type, bool tryEvade, int mod) {
	EobMonsterProperty *p = &_monsterProps[m->type];
	if (tryEvade) {
		if (tryMonsterAttackEvasion(m) || (p->capsFlags & 0x10))
			return true;
	}

	if (checkUnkConstModifiers(m, 0, p->level, mod, 6))
		return false;
	
	int para = 0;
		
	switch (type) {
	case 0:
	case 1:
	case 2:
		para = (type == 0) ? ((p->typeFlags & 1) ? 1 : 0) : ((type == 1) ? ((p->typeFlags & 2) ? 1 : 0) : 1);
		if (para && !(p->statusFlags & 2)) {
			m->mode = 10;
			m->spellStatusLeft = 15;
		}

		break;

	case 3:
		if (!(p->statusFlags & 8))
			inflictMonsterDamage(m, 1000, true);
		break;

	case 4:
		inflictMonsterDamage(m, 1000, true);
		break;

	case 5:
		m->flags |= 0x20;
		_sceneUpdateRequired = true;
		break;

	case 6:
		if (!(p->statusFlags & 4) && m->mode != 7 && m->mode != 8 && m->mode != 10) {
			m->mode = 0;
			m->spellStatusLeft = 20;
			para = (getNextMonsterDirection(m->block, _currentBlock) ^ 4) >> 1;
			m->flags |= 8;
			walkMonsterNextStep(m, -1, para);
		}
		break;

	default:
		break;
	}

	return true;
}

bool EobCoreEngine::turnUndeadHit(EobMonsterInPlay *m, int hitChance, int casterLevel) {
	uint8 e = _turnUndeadEffect[_monsterProps[m->type].tuResist * 14 + MIN(casterLevel, 14)];

	if (e == 0xff) {
		calcAndInflictMonsterDamage(m, 0, 0, 500, 0x200, 5, 3);
	} else if (hitChance < e) {
		return false;
	} else {
		m->mode = 0;
		m->flags |= 8;
		m->spellStatusLeft = 40;
		m->dir = (getNextMonsterDirection(m->block, _currentBlock) ^ 4) >> 1;	
	}

	return true;
}

void EobCoreEngine::printWarning(const char* str) {
	_txt->printMessage(str);
	snd_playSoundEffect(79);
}

void EobCoreEngine::printNoEffectWarning() {
	printWarning(_magicStrings4[0]);
}

void EobCoreEngine::spellCallback_start_armor() {

}

void EobCoreEngine::spellCallback_start_burningHands() {

}

void EobCoreEngine::spellCallback_start_detectMagic() {

}

bool EobCoreEngine::spellCallback_end_detectMagic(EobFlyingObject*) {
	return true;
}

void EobCoreEngine::spellCallback_start_magicMissile() {
	launchMagicObject(_openBookChar, 0, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_magicMissile(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 1, 4, 1, (getCharacterMageLevel(fo->attackerId) - 1) >> 1);
}

void EobCoreEngine::spellCallback_start_shockingGrasp() {

}

bool EobCoreEngine::spellCallback_end_shockingGraspFlameBlade(EobFlyingObject*) {
	return true;
}

void EobCoreEngine::spellCallback_start_improvedIdentify() {

}

void EobCoreEngine::spellCallback_start_melfsAcidArrow() {
	launchMagicObject(_openBookChar, 1, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_melfsAcidArrow(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 2, 4, 0, getCharacterMageLevel(fo->attackerId) / 3);
}

void EobCoreEngine::spellCallback_start_dispelMagic() {

}

void EobCoreEngine::spellCallback_start_fireball() {
	launchMagicObject(_openBookChar, 2, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_fireball(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 1, 6, 0, getCharacterMageLevel(fo->attackerId));
}

void EobCoreEngine::spellCallback_start_flameArrow() {
	launchMagicObject(_openBookChar, 3, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_flameArrow(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 5, 6, 0, getCharacterMageLevel(fo->attackerId));
}

void EobCoreEngine::spellCallback_start_holdPerson() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 4 : 3, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_holdPerson(EobFlyingObject *fo) {
	bool res = false;
	
	if (_flags.gameID == GI_EOB2 && fo->curBlock == _currentBlock) {
		// party hit
		int numChar = rollDice(1, 4, 0);
		int charIndex = rollDice(1, 6, 0);
		for (int i = 0; i < 6 && numChar; i++) {
			if (testCharacter(charIndex, 3)) {
				statusAttack(charIndex, 4, _magicStrings8[1], 4, 5, 9, 1);
				numChar--;
			}
			charIndex = (charIndex + 1) % 6;
		}
		res = true;

	} else {
		// monster hit
		for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, 1, 1); *m != -1; m++)
			res |= magicObjectStatusHit(&_monsters[*m], 0, true, 4);
	}

	return res;
}

void EobCoreEngine::spellCallback_start_lightningBolt() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 5 : 4, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_lightningBolt(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 1, 6, 0, getCharacterMageLevel(fo->attackerId));
}

void EobCoreEngine::spellCallback_start_vampiricTouch() {

}

bool EobCoreEngine::spellCallback_end_vampiricTouch(EobFlyingObject*) {
	return true;
}

void EobCoreEngine::spellCallback_start_fear() {
	sparkEffectOffensive();
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block == bl)
			magicObjectStatusHit(&_monsters[i], 6, true, 4);
	}
}

void EobCoreEngine::spellCallback_start_iceStorm() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 6 : 5, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_iceStorm(EobFlyingObject *fo) {
	static int8 blockAdv[] = { -32, 32, 1, -1 };
	bool res = magicObjectDamageHit(fo, 1, 6, 0, getCharacterMageLevel(fo->attackerId));
	if (res) {
		for (int i = 0; i < 4; i++) {
			uint16 bl = fo->curBlock;
			fo->curBlock = (fo->curBlock + blockAdv[i]) & 0x3ff;
			magicObjectDamageHit(fo, 1, 6, 0, getCharacterMageLevel(fo->attackerId));
			fo->curBlock = bl;
		}
	}
	return res;
}

void EobCoreEngine::spellCallback_start_removeCurse() {

}

void EobCoreEngine::spellCallback_start_coneOfCold() {

}

void EobCoreEngine::spellCallback_start_holdMonster() {

}

bool EobCoreEngine::spellCallback_end_holdMonster(EobFlyingObject *fo) {
	bool res = false;
	for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, 1, 1); *m != -1; m++)
		res |= magicObjectStatusHit(&_monsters[*m], 1, true, 4);
	return res;
}

void EobCoreEngine::spellCallback_start_wallOfForce() {

}

void EobCoreEngine::spellCallback_start_disintegrate() {

}

void EobCoreEngine::spellCallback_start_fleshToStone() {
	sparkEffectOffensive();
	int t = getClosestMonsterPos(_openBookChar, calcNewBlockPosition(_currentBlock, _currentDirection));
	if (t != -1)
		magicObjectStatusHit(&_monsters[t], 5, true, 4);
	else
		printWarning(_magicStrings8[4]);
}

void EobCoreEngine::spellCallback_start_stoneToFlesh() {
	if (_characters[_activeSpellCaster].flags & 8)
		_characters[_activeSpellCaster].flags &= ~8;
	else
		printNoEffectWarning();
}

void EobCoreEngine::spellCallback_start_trueSeeing() {
	_wllVmpMap[46] = 0;
}

bool EobCoreEngine::spellCallback_end_trueSeeing(EobFlyingObject*) {
	_wllVmpMap[46] = 1;
	return true;
}

void EobCoreEngine::spellCallback_start_slayLiving() {

}

void EobCoreEngine::spellCallback_start_powerWordStun() {

}

void EobCoreEngine::spellCallback_start_causeLightWounds() {

}

void EobCoreEngine::spellCallback_start_cureLightWounds() {
	modifyCharacterHitpoints(_activeSpellCaster, rollDice(1, 8));
}

void EobCoreEngine::spellCallback_start_aid() {

}

bool EobCoreEngine::spellCallback_end_aid(EobFlyingObject*) {
	return true;
}

void EobCoreEngine::spellCallback_start_flameBlade() {

}

void EobCoreEngine::spellCallback_start_slowPoison() {

}

bool EobCoreEngine::spellCallback_end_slowPoison(EobFlyingObject*) {
	return true;
}

void EobCoreEngine::spellCallback_start_createFood() {
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		_characters[i].food = 100;
	}
}

void EobCoreEngine::spellCallback_start_removeParalysis() {
	int numChar = 4;
	for (int i = 0; i < 6; i++) {
		if (!(_characters[i].flags & 4) || !numChar)
			continue;
		_characters[i].flags &= ~4;
		numChar--;
	}
}

void EobCoreEngine::spellCallback_start_causeSeriousWounds() {
	
}

void EobCoreEngine::spellCallback_start_cureSeriousWounds() {
	modifyCharacterHitpoints(_activeSpellCaster, rollDice(2, 8, 1));
}

void EobCoreEngine::spellCallback_start_neutralizePoison() {

}

void EobCoreEngine::spellCallback_start_causeCriticalWounds() {

}

void EobCoreEngine::spellCallback_start_cureCriticalWounds() {
	modifyCharacterHitpoints(_activeSpellCaster, rollDice(3, 8, 3));
}

void EobCoreEngine::spellCallback_start_flameStrike() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 8 : 7, _currentBlock, _activeSpellCasterPos, _currentDirection);
}

bool EobCoreEngine::spellCallback_end_flameStrike(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 6, 8, 0, 0);
}

void EobCoreEngine::spellCallback_start_raiseDead() {

}

void EobCoreEngine::spellCallback_start_harm() {

}

void EobCoreEngine::spellCallback_start_heal() {
	EobCharacter *c = &_characters[_activeSpellCaster];
	if (c->hitPointsMax <= c->hitPointsCur)
		printWarning(_magicStrings4[0]);
	else
		modifyCharacterHitpoints(_activeSpellCaster, c->hitPointsMax - c->hitPointsCur);
}

void EobCoreEngine::spellCallback_start_layOnHands() {
	modifyCharacterHitpoints(_activeSpellCaster, _characters[_openBookChar].level[0] << 1);
}

void EobCoreEngine::spellCallback_start_turnUndead() {
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	if (!(_levelBlockProperties[bl].flags & 7))
		return;
	
	int cl = _openBookCasterLevel ? _openBookCasterLevel : getCharacterClericPaladinLevel(_openBookChar);
	int r = rollDice(1, 20);
	bool hit = false;

	for (const int16 *m = findBlockMonsters(bl, 4, 4, 1, 1); *m != -1; m++) {		
		if ((_monsterProps[_monsters[*m].type].typeFlags & 4) && !(_monsters[*m].flags & 0x10)) {
			_preventMonsterFlash = true;
			_monsters[*m].flags |= 0x10;
			hit |= turnUndeadHit(&_monsters[*m], r, cl);
		}
	}

	if (hit) {
		turnUndeadAutoHit();
		snd_playSoundEffect(95);
		updateAllMonsterShapes();
	}

	_preventMonsterFlash = false;
}

bool EobCoreEngine::spellCallback_end_unk1Passive(EobFlyingObject *fo) {
	bool res = false;
	if (_partyEffectFlags & 0x20000) {
		res = magicObjectDamageHit(fo, 4, 10, 6, 0);
		if (res) {
			gui_drawAllCharPortraitsWithStats();
			_partyEffectFlags &= ~0x20000;
		}
	} else {
		res = magicObjectDamageHit(fo, 12, 10, 6, 0);
	}
	return res;
}

bool EobCoreEngine::spellCallback_end_unk2Passive(EobFlyingObject *fo) {
	return magicObjectDamageHit(fo, 0, 0, 18, 0);
}

bool EobCoreEngine::spellCallback_end_deathSpellPassive(EobFlyingObject *fo) {
	return true;
}

bool EobCoreEngine::spellCallback_end_disintegratePassive(EobFlyingObject *fo) {
	return true;
}

bool EobCoreEngine::spellCallback_end_causeCriticalWoundsPassive(EobFlyingObject *fo) {
	return true;
}

bool EobCoreEngine::spellCallback_end_fleshToStonePassive(EobFlyingObject *fo) {
	return true;
}

}	// End of namespace Kyra

#endif // ENABLE_EOB
